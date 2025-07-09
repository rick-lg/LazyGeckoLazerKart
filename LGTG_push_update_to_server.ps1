ssh root@192.168.69.1 "cp -r /root/lg_tg/ota/ /root/lg_tg/ota_old/; rm -rf /root/lg_tg/ota/*"
scp -r -O "C:\GitLG\LazyGeckoLazerKart\builds\*" root@192.168.69.1:/root/lg_tg/ota