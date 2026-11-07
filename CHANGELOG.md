# 変更履歴

このファイルでは DCC Monitor の主な変更内容を記録します。

---

## v1.0.0

初回公開版

### 基本機能

- Raspberry Pi Pico を使用した DCC信号受信
- DCCビット復元
- DCCパケット復元
- パケットモニタ表示
- パケット通番表示
- パケット間隔表示

### パケット解析

- CRCチェック
- Reset Packet解析
- Idle Packet解析
- Loco Packet解析
- Accessory Packet解析
- Service Mode Packet解析
- OPS Mode Packet解析

### メニューシステム

- 階層型メニュー
- ESCによる画面遷移
- パケット監視画面
- パケット統計画面
- 信号統計画面
- 設定画面

### フィルタ機能

- パケット種別フィルタ
  - ALL
  - LOCO
  - ACCESSORY
  - OPS

- アドレスフィルタ

### 統計機能

- 総受信パケット数
- Reset Packet数
- Idle Packet数
- Loco Packet数
- Accessory Packet数
- Service Packet数
- OPS Packet数
- CRC Error数

### 信号解析機能

- Bit0カウント
- Bit1カウント
- Bit0パルス幅統計
- Bit1パルス幅統計
- パルス幅ヒストグラム
- Bit0 Mode値算出
- Bit1 Mode値算出

### 設定機能

- Bit判定閾値変更
- プリアンブル判定数変更
- Reset Packet表示ON/OFF
- Idle Packet表示ON/OFF

