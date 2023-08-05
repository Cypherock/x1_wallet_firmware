#!/usr/bin/env bash
EXIT_STATUS=1

if [[ $# -lt 2 ]]; then
    echo -e "USAGE: $0 <BASE_BRANCH_COMMIT_ID> <WORK_BRANCH_COMMIT_ID>"
    exit ${EXIT_STATUS}
fi

BASE_BRANCH_SHA=$1
WORK_BRANCH_SHA=$2

# generate list of commits in the that will be merged in the base branch
# Ref: https://git-scm.com/book/en/v2/Git-Tools-Revision-Selection#double_dot
COMMIT_LIST=$(git log --format=%H $BASE_BRANCH_SHA..$WORK_BRANCH_SHA)
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