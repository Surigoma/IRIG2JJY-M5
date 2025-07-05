[English README is here](README.md)

# IRIG2JJY-M5

M5Stack (M5StickC Plus) を用いて IRIG-B 時刻信号をデコードし、ラジオ時計向けの JJY 信号を生成するファームウェアです。リアルタイムクロック同期や LCD デバッグ表示、マルチタスク・割り込み安全設計を備えています。

## 特徴

- IRIG 信号（IRIG-B）のデコード
- JJY 信号の生成
- リアルタイムクロック同期
- LCD によるデバッグ表示
- マルチタスク・割り込み安全設計

## ハードウェア要件

- M5Stack M5StickC Plus（または互換品）
- IRIG 信号入力（GPIO36）
- JJY 出力（GPIO26）
- LED 出力（GPIO10）

## はじめかた

1. **本リポジトリをクローン**
2. **PlatformIO（推奨）または Arduino IDE で開く**
3. **ボード設定を M5StickC Plus に合わせる**
4. **必要なライブラリをインストール**（例: M5StickCPlus, ArduinoJson など。必要に応じてプロンプトが表示されます）
5. **ビルドして書き込み**

## ファイル構成

- `src/` : メインソースコード
- `include/` : ヘッダファイル
- `lib/` : 外部ライブラリ（必要に応じて）
- `test/` : ユニットテストコード（PlatformIO/Unity）

## 主な構成要素

- `main.cpp` : エントリポイント、マルチタスク設定、メインループ
- `IRIG.hpp/cpp` : IRIG 信号デコーダ
- `JJY.hpp/cpp` : JJY 信号ジェネレータ
- `clockManager.hpp/cpp` : クロック・タイミング管理
- `define.hpp` : ピン定義

## 使い方

- IRIG 入力をデコードし、システムクロックを同期します。
- 同期した時刻に基づき JJY 出力を生成します。
- デバッグ情報は LCD に表示されます。
- ボタン A を押すとデバイスがリセットされます。

## テスト

- IRIG, JJY, clockManager のユニットテストは `test/` ディレクトリにあります。
- PlatformIO でテストを実行するには:
  ```
  pio test
  ```

## ドキュメント生成

- 主要なクラスや関数は Doxygen 形式でドキュメント化されています。
- API ドキュメントを生成するには:
  ```
  doxygen
  ```
  （Doxygen のインストールが必要です）

## コントリビュート

バグ報告やプルリクエストを歓迎します。質問や提案は issue をご利用ください。

## ライセンス

MIT License (c) 2025 Surigoma
