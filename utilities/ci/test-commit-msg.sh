#!/bin/bash
EXIT_STATUS=1

if [[ $# -lt 2 ]]; then
    echo -e "USAGE: $0 <BASE_COMMIT_ID> <HEAD_COMMIT_ID>"
    exit ${EXIT_STATUS}
fi

BASE_SHA=$1
HEAD_SHA=$2
COMMIT_LIST=$(git log --format=%H $BASE_SHA...$HEAD_SHA)
EXIT_STATUS=0

for COMMIT in $COMMIT_LIST; do
    git show --pretty=format:"%B" --no-patch $COMMIT > message
    echo -e "\n\n------------------------"
    echo -e "Working with commit message:\n"
    cat message
    echo -e ""
    bash ./utilities/hooks/commit-msg message
    if [ $? -gt 0 ]; then
        EXIT_STATUS=1
    fi
done

exit $EXIT_STATUS