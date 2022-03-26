## Issue priority level

#### There are three levels of priority for issues and their related changes (MR). These are (with criteria determines which changes fall into this level of priority):

- **Priority 1**
  - All changes that effect a large part of the codebase (`large LOC; > 250`)
  - New feature implementation
  - Changes that effect more than one application flow
- **Priority 2**
  - All changes with moderate LOC (`> 50 & < 250`)
  - Changes that affect only one segment of the application
  - Minor changes effecting significant change in application logic
- **Priority 3**
  - Very few LOC (`< 50`)
  - Small bug/typo fixes
  - Changes that do not do a drastic modification of application logic


### MR review approach for priority levels

#### Following points apply to all MRs:
- The priority level of each MR should be clearly mentioned on the Gitlab and Asana
- Reviewer/Author can change the priority level if necessary
- If any of the changes has been tested by the Author it is suggested to **add the code used for testing**. This will help fasten the review process and quickly discover unhandled corner cases.

#### Review approach note
- **Priority 1**
  - Author should post the tested cases and test-code used for testing out the changes
  - At lest 2 reviewers (other than the author) should do a review of the code
  - Testing of corner cases should be done and update on the Gitlab (suggested: for both reviewers independently)
  - Testing should include tests on physical device
- **Priority 2**
  - Author should post the tested cases and test-code used for testing out the changes (if any)
  - Two reviewers (other than the author) should do a review of the code
  - Testing of corner cases can be done and update on the Gitlab (suggested: at least from one reviewer independently)
- **Priority 3**
  - One reviewer (other than the author) should do a review of the code
  - Testing can be done on an as-needed basis

## Generating signed binaries (Using the bash script)
### Windows setup steps
1. Install the arm-build-tools [download link](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07win32/gcc-arm-none-eabi-10.3-2021.07-win32.exe)<br>
**NOTE: After installation, check the 'Add to environment PATH variable' option**
2. clone the cli tool from the repo and **checkout to `new-header` branch**.
3. Build the cli-tool ready by issuing following commands
   - `npm i -g yarn`
   - `yarn && yarn build`
4. copy all the `.h` and `-version.txt` files from [this repo](https://gitlab.com/cypherock-tech/privatekeypairs) into `communication-testing` folder
5. Download the ninja release `zip file` for your specific platform from [here](https://github.com/ninja-build/ninja/releases) (Suggesting ninja for faster build)
6. Extract the zip file and copy `ninja` file inside the `bin` folder of arm-build-tools `installation path`
7. run the build command as follows: `./stm-build.sh`

### Linux (Ubuntu) setup steps
1. Install the required tools. Use following apt command: `apt-get install -y newlib-arm-none-eabi gcc-arm-none-eabi ninja-build cmake `
2. clone the cli tool from the repo and **checkout to `new-header` branch**.
3. Build the cli-tool ready by issuing following commands
   - `npm i -g yarn`
   - `yarn && yarn build`
4. copy all the `.h` and `-version.txt` files from [this repo](https://gitlab.com/cypherock-tech/privatekeypairs) into `communication-testing` folder
5. run the build command as follows: `./stm-build.sh`

## Code style guidelines
`clang` is prefered for checking the code-styling.<br/>
This project is expected to follow the Chromium config of clang tool (with following modifications).

- pointer alignment = right
- indentation = 2 (default)


## Code Commenting Guidelines

### Comment style

We use Doxygen style comments for our codebase using at least the following tags where ever applicable.

#### **Multiline comment format**
```c
/**
 * @brief <Add a brief description here>
 * 
 * <Add a detailed description here if necessary>
 * 
 * @param <Add parameter name here> <Add a line explaining what this parameter means>
 * @param[in] <Add parameter name here> <Add a description about the input parameter here>
 * @param[out] <Add parameter name here> <Add a description about the output parameter of function that returns multiple values>
 * @param[in, out] <Add parameter name here> <Add a description about the parameter used for both input and output in a function here>
 * @return <Add return type here> <Add a description of return value of this function>
 * @retval <Add return one of the value here> <Add a description about the returned value>
 * 
 * @note <Add a note about the this function, this may be a note to the devs or something to keep in mind>
 * @warning <Add a warning here if necessary>
 * @see <Add some function name that devs should see>
 * 
 * @remark <Additional note if necessary>
 */
```

#### **Single line comment**

```c
/// <Add a single line description here>
```

> **NOTE:** Comments for particular entity should be added _**ONLY**_ once at the location where they are declared. 
> `extern` variables should _**NOT**_ be commented every time they are referenced.

> **NOTE:** All functions declared in a header file must be commented only in the header file using the style mentioned above.
> Functions that are not declared in a header file should be commented in the source file. Make sure that the functions are not commented twice.
