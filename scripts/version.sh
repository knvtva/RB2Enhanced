if git --version &>/dev/null; then
# if git is installed, get the current information from the repository
    [[ ! -z ${GITHUB_ACTIONS} ]] && [[ ${RUNNER_OS} == "Linux" ]] && git config --global --add safe.directory /__w/RB2Enhanced/RB2Enhanced
    cat > source/version.h <<EOF
// This file will be auto generated during building, and should
// not be modified manually
// The git tag used to compile RB2Enhanced
#define RB2E_BUILDTAG "$(git describe --always --tags --dirty)"
// The git branch and commit used to compile RB2Enhanced
#define RB2E_BUILDCOMMIT "$(git rev-parse --abbrev-ref HEAD)-$(git describe --always --match \"tag\" --dirty)"
EOF
else
# if git isn't installed, we don't know the version, use the compile time in logs
    cat > source/version.h <<EOF
// This file will be auto generated during building, and should
// not be modified manually
// The git tag used to compile RB2Enhanced
#define RB2E_BUILDTAG "Unknown"
// The git branch and commit used to compile RB2Enhanced
#define RB2E_BUILDCOMMIT "$(date) @ $HOSTNAME"
EOF
fi