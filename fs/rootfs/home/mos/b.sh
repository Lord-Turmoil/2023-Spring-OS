# Demonstrate ; and &

echo "Test B: Demonstration of ; and &"

echo "/home/mos/timer 3000 aaa; /home/mos/timer 2000 bbb; /home/mos/timer 1000 aaa"
/home/mos/timer 3000 aaa; /home/mos/timer 2000 bbb; /home/mos/timer 1000 aaa

echo "/home/mos/timer 3000 aaa& /home/mos/timer 2000 bbb& /home/mos/timer 1000 aaa"
/home/mos/timer 3000 aaa& /home/mos/timer 2000 bbb& /home/mos/timer 1000 aaa

exit