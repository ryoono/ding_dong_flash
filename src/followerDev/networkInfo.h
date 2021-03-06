#ifndef _NETWORKINFO_H
#define _NETWORKINFO_H

// IPアドレス情報
#define LEADER_IP_ADDRESS     192,168,1,1     // 親機のIPアドレス
#define FOLLOWER_IP_ADDRESS_1 192,168,1,2     // 子機1のIPアドレス
#define FOLLOWER_IP_ADDRESS_2 192,168,1,3     // 子機2のIPアドレス
#define FOLLOWER_IP_ADDRESS_3 192,168,1,4     // 子機3のIPアドレス
#define MY_SUBNETMASK         255,255,255,248 // サブネットマスク

// ポート番号情報
#define LEADER_PORT_TX1       8080            // 親機ポート番号(子機1への送信用)
#define LEADER_PORT_TX2       8081            // 親機ポート番号(子機2への送信用)
#define LEADER_PORT_TX3       8082            // 親機ポート番号(子機3への送信用)

#define FOLLOWER_PORT_RX      8081            // 子機ポート番号(親機からの受信用)

// デバイスID
#define FOLLOWER_ID_1 '1' // 子機1のIPアドレス
#define FOLLOWER_ID_2 '2' // 子機2のIPアドレス
#define FOLLOWER_ID_3 '3' // 子機3のIPアドレス

#endif
