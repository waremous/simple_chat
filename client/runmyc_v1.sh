#/bin/bash

rm client.out
cc -std=c11 -g $1
if (($?))
then
      echo ERROR && exit 1
fi
mv a.out client.out
./client.out



