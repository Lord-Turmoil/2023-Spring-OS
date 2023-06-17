# Create a complex directory under current directory

echo "Test A: Creating Complex Directory"

echo -n "Current directory: "
pwd

echo "----- Creating Directories -----"
mkdir -p -v aaaaa/bbbbb/ccccc/../CCCCC/DDDDD/../../ccccc/ddddd
echo "This is directory bbbbb" > aaaaa/bbbbb.txt
echo "This is directory DDDDD" > aaaaa/bbbbb/CCCCC/DDDDD.txt
echo "This is directory ddddd" > aaaaa/bbbbb/ccccc/ddddd.txt
echo "--------------------------------"

tree aaaaa # Display result

exit # Not necessary
