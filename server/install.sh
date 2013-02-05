#!/bin/sh
rm /usr/lib/libldperfect_util.so
rm /usr/lib/libldperfect_net.so

cp util_lib/libldperfect_util.so  /usr/lib
cp net_lib/libldperfect_net.so /usr/lib

rm bin/mysql_server
rm bin/login_server
rm bin/game_server
rm bin/gate_server

cp mysql_server/mysql_server bin/
cp login_server/login_server bin/
cp game_server/game_server bin/
cp gate_server/gate_server bin/

cp account_server/login /usr/lib/cgi-bin/client
cp account_server/register /usr/lib/cgi-bin/client
cp account_server/guest /usr/lib/cgi-bin/client/guest/register
cp account_server/bind /usr/lib/cgi-bin/client/guest/bind

cp account_server/question /usr/lib/cgi-bin/client/question/post/
cp account_server/question_list /usr/lib/cgi-bin/client/question/list/

cp account_server/share_get /usr/lib/cgi-bin/client/share/share_get
cp account_server/share_list /usr/lib/cgi-bin/client/share/share_list
cp account_server/share_share /usr/lib/cgi-bin/client/share/share_share
cp account_server/share_share2 /usr/lib/cgi-bin/client/share/share_share2
