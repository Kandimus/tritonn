if [ -z "$1" ]; then
	echo "json.sh <json req>"
else
	head -c -1 $1 > $1".bin"
	socat -t 2 - tcp4:127.0.0.1:22501 > answe.txt < $1".bin"
	cat answe.txt
	echo -e
	rm $1".bin"
fi

