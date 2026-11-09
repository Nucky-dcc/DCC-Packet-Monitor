# DCC Packet Monitor

Real-time NMRA DCC Packet Analyzer for Raspberry Pi Pico (RP2040)

DCC Packet Monitor は、RP2040 (Raspberry Pi Pico) を使用した
NMRA DCC信号解析ツールです。

DCCパケットをリアルタイムに解析し、

- DCCデコーダ開発
- コマンドステーション検証
- Programming Track解析
- OPS Mode解析
- DCC学習
- 信号品質評価

を支援します。

本プロジェクトは GPLv3 ライセンスで公開されています。

---

# Features

## Packet Monitoring

リアルタイムでDCCパケットを表示します。

表示内容

- パケット通番
- 前パケットからの経過時間
- 生パケットデータ
- CRCチェック結果
- 解析結果

表示例

```text
000123  028.415ms  03 14 10 07     | Loco:3 [128SPD] FWD 7
000124  005.812ms  FF 00 FF        | Idle
000125  012.003ms  80 F1 70        | Acc:1 Straight
```

---

## Packet Filter

以下のフィルタに対応しています。

- ALL
- Loco
- Loco (Speed)
- Loco (Function)
- Accessory
- OPS

さらにアドレスフィルタが利用できます。

```text
Address Filter = 3
```

のように設定すると、
指定アドレスのみ表示できます。

---

## Locomotive Packet Analysis

対応内容

- Short Address
- Long Address
- 128 Speed Step
- Function F0-F4
- Function F5-F8
- Function F9-F12
- Function F13-F68
- Advanced Consist Detection

表示例

```text
Loco:123 [128SPD] FWD 45
```

---

## Accessory Packet Analysis

Basic Accessory Decoder Packet を解析します。

表示内容

- アクセサリアドレス
- Closed Route（Straight）
- Thrown Route（Diverging）

例

```text
Acc:12 Closed Route（Straight）
Acc:12 Thrown Route（Diverging）
```

---

## Programming Track Analysis

Service Mode Packet を解析します。

対応内容

### Direct Byte Mode

```text
Prog Direct Byte CV1 Write=3
```

### Direct Bit Mode

```text
Prog Direct Bit CV29 Write Bit0=1
```

### Paged Mode

```text
Prog PageReg=5
Prog Paged CV17 Write=192
```

---

## OPS Mode Analysis

Main Track Programming (Operations Mode) を解析します。

表示内容

- ターゲットアドレス
- CV番号
- 書込み値

例

```text
OPS Addr3 CV1 Write=10
```

---

## Signal Analysis

DCC 0/1信号の統計解析機能を搭載しています。

表示内容

- Bit Rate
- Bit0数
- Bit1数
- Bit0最頻値
- Bit1最頻値
- Pulse Histogram

例

```text
Bit1 Mode : 58us
Bit0 Mode : 116us
```

---

## Histogram

0～300us を1us単位で統計表示します。

例

```text
58 us : 1452
116 us : 987
```

コマンドステーションの波形品質評価に利用できます。

---

# System Architecture

```text
GPIO Input
    │
    ▼
dcc_rx
    │
    ▼
dcc_decode
    │
    ▼
dcc_packet
    │
    ▼
packet_analyzer
    ├─ loco_analyzer
    ├─ accessory_analyzer
    ├─ service_analyzer
    └─ ops_analyzer
```

設計方針

- 受信
- Bit判定
- Packet復元
- Packet解析

を分離し、
保守性と教育的価値を重視しています。

---

# Hardware

開発環境

- Raspberry Pi Pico
- RP2040

入力信号

- NMRA DCC Track Signal

現在のデフォルト入力ピン

```text
GPIO2
```

---

# Build

必要環境

- Pico SDK 2.3.1
- CMake 3.13以上
- ARM GCC Toolchain

ビルド

```bash
mkdir build
cd build
cmake ..
make
```

生成ファイル

```text
DCC_Packet_Monitor.uf2
```

---

# License

Copyright (C) 2026 Nucky

This project is licensed under the GNU General Public License Version 3.

See LICENSE for details.

SPDX-License-Identifier: GPL-3.0-only

---

# Disclaimer

本ソフトウェアは開発支援および学習用途を目的としています。

使用によって発生した損害について作者は責任を負いません。

---

# Acknowledgements

- NMRA (National Model Railroad Association)
- Raspberry Pi Foundation
- Raspberry Pi Pico SDK