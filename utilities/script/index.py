# Requires ecdsa v0.16.1
import os
import re
import math
import argparse
import re
import struct
from zlib import crc32
import hashlib
from ecdsa import SigningKey, NIST256p

DEFAULT_BIN_PREFIX = 'BlinkLed'

ADD_HEADER_DEFAULT_INPUT_BIN = f'{DEFAULT_BIN_PREFIX}.bin'
ADD_HEADER_DEFAULT_OUTPUT_BIN = f'{DEFAULT_BIN_PREFIX}_Header.bin'
ADD_HEADER_DEFAULT_VERSION_FILE = f'{DEFAULT_BIN_PREFIX}-version.txt'

SIGN_HEADER_DEFAULT_INPUT_BIN = f'{DEFAULT_BIN_PREFIX}_Header.bin'
SIGN_HEADER_DEFAULT_OUTPUT_BIN = f'{DEFAULT_BIN_PREFIX}_Signed.bin'

parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(help='sub-command help', dest="action")

parser_gen = subparsers.add_parser('gen-key', help='Generate the first key pairs')
parser_gen.add_argument("--private-key", help="Path to the save the private key file. Defaults to `private_key`", default="private_key")
parser_gen.add_argument("--public-key", help="Path to the save the public key file. Defaults to `public_key`", default="public_key")
parser_gen.add_argument("--index", help="Index of the key pairs. Defaults to `1`", type=int, default="1")

parser_add_header = subparsers.add_parser('add-header', help='Add headers to the binary file')
parser_add_header.add_argument("--private-key", help="Path to the private key file. Defaults to `private_key1.h`", default="private_key1.h")
parser_add_header.add_argument("--input", help=f"Path to the input bin file. Defaults to `{ADD_HEADER_DEFAULT_INPUT_BIN}`", default=ADD_HEADER_DEFAULT_INPUT_BIN)
parser_add_header.add_argument("--output", help=f"Path to the output bin file. Defaults to `{ADD_HEADER_DEFAULT_OUTPUT_BIN}`", default=ADD_HEADER_DEFAULT_OUTPUT_BIN)
parser_add_header.add_argument("--version", help=f"Path to the version file. Defaults to `{ADD_HEADER_DEFAULT_VERSION_FILE}`", default=ADD_HEADER_DEFAULT_VERSION_FILE)

parser_sign = subparsers.add_parser('sign-header', help='Sign the binary file')
parser_sign.add_argument("--private-key", help="Path to the private key file. Defaults to `private_key2.h`", default="private_key2.h")
parser_sign.add_argument("--input", help=f"Path to the input bin file. Defaults to `{SIGN_HEADER_DEFAULT_INPUT_BIN}`", default=SIGN_HEADER_DEFAULT_INPUT_BIN)
parser_sign.add_argument("--output", help=f"Path to the output bin file. Defaults to `{SIGN_HEADER_DEFAULT_OUTPUT_BIN}`", default=SIGN_HEADER_DEFAULT_OUTPUT_BIN)

parser_decode_header = subparsers.add_parser('decode-header', help='Decodes headers of the binary file')
parser_decode_header.add_argument("--input", help=f"Path to the input bin file. Defaults to `{SIGN_HEADER_DEFAULT_OUTPUT_BIN}`", default=SIGN_HEADER_DEFAULT_OUTPUT_BIN)

args = parser.parse_args()

ACTION = args.action
OUTPUT_DIR = os.path.join("../", "output")

def bigToLittleEndian(s):
    newStr = ''
    for i in range(int((len(s)) / 2)):
        newStr = s[i * 2: i* 2+2] + newStr

    return newStr

def bytesToCBytesStr(bytesStr):
    if (type(bytesStr) != bytes):
        raise Exception('`bytesToCBytes` require an argument of type `bytes`.')
    s = [];
    for elem in bytesStr:
        s.append(hex(elem))
    return ', '.join(s)

def readVersionFile(filePath):
    # NOTE: The Version file should contain data in integer (decimal) format
    fw_ver_str = ''
    hw_ver_str = ''
    magic_no = ''
    with open(filePath) as f:
        for line in f:
            (key, val) = line.split('=', 1)
            if key == 'firmware version':
                fw_ver_str = val.strip()
            if key == 'hardware version':
                hw_ver_str = val.strip()
            if key == 'magic number':
                magic_no = val.strip()

    if (fw_ver_str == ''):
        raise Exception('Firmware version not found')
    if (hw_ver_str == ''):
        raise Exception('Hardware version not found')
    if (magic_no == ''):
        raise Exception('Magic number not found')

    print ("firmware version: {}".format(fw_ver_str))
    print ("hardware version: {}".format(hw_ver_str))
    print ("magic number: {}".format(magic_no))
    str_versions = [fw_ver_str, hw_ver_str]
    num_versions = []
    for ver in str_versions:
        ver_num = re.split(':', ver)
        version_num = int(ver_num[0]) << 24 | int(ver_num[1]) << 16 | int(ver_num[2]) << 8 | int(ver_num[3])
        num_versions.append(struct.pack("<I", version_num))
    return (num_versions, magic_no)

def readKeyFromFile(filename):
    with open(filename, 'r') as f:
        content = f.read().splitlines()
        f.close()
        if (len(content) < 11):
            raise Exception('Invalid file {filename}'.format(filename))
        keyLine = content[6]
        keyLineArr = keyLine.split("=")
        if (len(keyLineArr) < 2):
            raise Exception('Invalid line no 7 on file {filename}'.format(filename))
        hexStr = keyLineArr[1]
        return hexStr

def writeCFile(filepath, varName, varLength, commentVarName, binaryData, hexData):
    cStr = bytesToCBytesStr(binaryData)
    l = 0

    for elem in cStr:
        if elem == ',':
            l+=1

    mid = math.ceil(l / 2)

    l = 0
    midPoint = -1 
    
    for i in range(len(cStr)):
        elem = cStr[i]
        if elem == ',':
            l+=1
            if l == mid:
                midPoint = i + 2
                break

    formattedCStr = cStr;
    if (midPoint != -1): 
        formattedCStr = "\t" + cStr[:midPoint - 1] + "\n\t" + cStr[midPoint:]

    with open(filepath, 'w') as f:
        f.write('/*\n')
        f.write(' * ******************* WARNING ******************\n')
        f.write(' * This is an auto generated file, don\'t make any changes.\n')
        f.write(' * Otherwise this will not be compatable with the CLI.\n')
        f.write(' */\n')
        f.write('\n')
        f.write('// {var}={key}\n'.format(var=commentVarName, key=hexData))
        f.write('const uint8_t {}[{}] =\n'.format(varName, varLength))
        f.write('{\n')
        f.write('{}\n'.format(formattedCStr))
        f.write('};')
        f.close()

def addHeader(privateKeyFile, customInput, customOutput, versionFilePath):
    header = []
    filePath = customInput
    filename = os.path.basename(filePath)
    filesize = os.path.getsize(filePath)
    (num_versions, magic_no) = readVersionFile(versionFilePath)

    # Remove the initial 0x
    filesizeHex = hex(filesize)[2:]

    # Pad with 0 to create a 4 byte hex
    filesizeHex = bigToLittleEndian(filesizeHex.zfill(8))
    
    f1 = open(filePath, 'rb')
    filedata = f1.read()
    signatureData = ''
    privateKey=bytes.fromhex(readKeyFromFile(privateKeyFile))
    sk1 = SigningKey.from_string(privateKey, curve=NIST256p, hashfunc=hashlib.sha256)
    sig = sk1.sign_deterministic(filedata)
    signatureData += sig.hex();

    # Add dummy data for second sig
    for i in range(128):
        signatureData += '0'

    crc_int = crc32(filedata)
    crc_value = struct.pack("<I", crc_int)
    metadata = bytes.fromhex(filesizeHex) + bytes.fromhex(magic_no) + num_versions[0] + num_versions[1] + crc_value
    header[0:] = metadata
    for i in range(64 - len(metadata)):
        header.append(0)

    headerSig = sk1.sign_deterministic(bytes(header))
    header[64:] = headerSig;


    for i in range(128 - len(header)):
        header.append(0)

    outputFilePath = customOutput

    binaryHeader = bytes('', 'utf-8')
    for elem in header:
        binaryHeader += elem.to_bytes(1, 'big')
    binarySignature = bytes.fromhex(signatureData)

    f2 = open(outputFilePath, 'wb')
    f2.write(binaryHeader)
    f2.write(binarySignature)
    f2.write(filedata)
    f1.close()
    f2.close()

def addSig(privateKeyFile, customInput, customOutput):
    header = []
    filePath = customInput
    f1 = open(filePath, 'rb')
    allData = f1.read()
    headerData = allData[:128]
    signatureData = ''
    firstSig = allData[128:192]
    secondSig = allData[192:256]
    filedata = allData[256:]

    if (re.match("[^0]", secondSig.hex()) != None):
        raise Exception('The binary file already contains second signature.')

    sk1 = SigningKey.from_string(bytes.fromhex(readKeyFromFile(privateKeyFile)), curve=NIST256p, hashfunc=hashlib.sha256)
    sig = sk1.sign_deterministic(filedata)
    signatureData += sig.hex();

    outputFilePath = customOutput

    binarySignature = bytes.fromhex(signatureData)

    f2 = open(outputFilePath, 'wb')
    f2.write(headerData)
    f2.write(firstSig)
    f2.write(binarySignature)
    f2.write(filedata)
    f1.close()
    f2.close()

def decodeHeader(customInput):
    header = []
    filePath = customInput

    f1 = open(filePath, 'rb')

    allData = f1.read()
    headerData = allData[:64]
    headerSig = allData[64:128]
    firstSig = allData[128:192]
    secondSig = allData[192:256]
    # filedata = allData[:256]

    lastIndex = -1 
    filesize = ''
    magic_no = ''
    firmwareVersion = ''
    hardwareVersion = ''

    # Size is 4
    filesize = bigToLittleEndian((headerData[lastIndex + 1: lastIndex + 5]).hex())
    lastIndex += 4

    # Size is 4
    binaryMagicNo = headerData[lastIndex + 1: lastIndex + 5]
    lastIndex += 4

    # Size is 4
    binaryFV = headerData[lastIndex + 1: lastIndex + 5]
    lastIndex += 4

    # Size is 4
    binaryHV = headerData[lastIndex + 1: lastIndex + 5]
    lastIndex += 4

    # Size is 4
    binaryCRC = headerData[lastIndex + 1: lastIndex + 5]

    magic_no = binaryMagicNo.hex()
    firmwareVersion = bigToLittleEndian(binaryFV.hex())
    hardwareVersion = bigToLittleEndian(binaryHV.hex())
    crcValue = binaryCRC.hex()

    print("");
    print("Filesize: {}".format(filesize))
    print("Magic No: {}".format(magic_no))
    print("Firmware Version: {}".format(firmwareVersion))
    print("Hardware Version: {}".format(hardwareVersion))
    print("CRC Value: {}".format(crcValue))
    print("");
    print("Raw header data: {}".format(headerData.hex()))
    print("Header Sig: {}".format(headerSig.hex()))
    print("");
    print("First Sig: {}".format(firstSig.hex()))
    print("Second Sig: {}".format(secondSig.hex()))

def genKeys(privateKeyFile, publicKeyFile, keyIndex):
    sk = SigningKey.generate(curve=NIST256p, hashfunc=hashlib.sha256)
    vk = sk.verifying_key
    binarySk = sk.to_string()
    binaryVk = vk.to_string("compressed")

    hexSk = binarySk.hex()
    hexVk = binaryVk.hex()

    privateKeyVarName = f"private_key{keyIndex}";
    publicKeyVarName = f"public_key{keyIndex}";

    writeCFile(privateKeyFile, privateKeyVarName, '32', 'PRIVATE_KEY_HEX', binarySk, hexSk)
    writeCFile(publicKeyFile, publicKeyVarName, '33','PUBLIC_KEY_HEX', binaryVk, hexVk)

if (ACTION == 'gen-key'):
    genKeys(args.private_key + str(args.index) + '.h', args.public_key + str(args.index) + '.h', args.index)
elif (ACTION == 'add-header'):
    addHeader(args.private_key, args.input, args.output, args.version)
elif (ACTION == 'sign-header'):
    addSig(args.private_key, args.input, args.output)
else:
    decodeHeader(args.input)

print("Completed")
