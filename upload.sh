DEBUG_EXE=./debug/main
RELEASE_EXE=./release/main
REMOTE_ADDR=54.195.186.38
REMOTE_USER=ubuntu

if test -f "$DEBUG_EXE"; then
echo "uploading debug executable..."
scp "$DEBUG_EXE" $REMOTE_USER@$REMOTE_ADDR:~/debug
fi

if test -f "$RELEASE_EXE"; then
echo "uploading release executable..."
scp "$RELEASE_EXE" $REMOTE_USER@$REMOTE_ADDR:~/release
fi