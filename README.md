# Mimiker: 教育・研究用のUnixライクなシステム

Mimikerの主な目的は最小限のUnixライクなオペレーティングシステム、すなわち
カーネルと一連のユーザ空間プログラムを提供することです。

カーネルの設計はFreeBSDとNetBSDに大きくインスパイアされており、LinuxやPlan9、
その他のOSからもアイデアを取り入れています。私たちはオープンソースの
オペレーティングシステムのソースコードを読むことに多くの時間を費やしています。
そして、それらから最良の設計判断、アイデア、アルゴリズム、API、実践などを
注意深く選択して、最小限のものを抽出し、Mimikerのコードベースに再実装したり、
適応させたりしています。私たちは彼らの失敗は繰り返したくはありません、また、
レガシーで完璧でないソリューションから離れたいと願っています。

Mimikerプロジェクトにはコードのミニマリズム、シンプルさ、読みやすさを重視する
考えを持つ人々が集まっています。私たちは可能な限り複雑さを抑えたソリューション
を目指しています。有用でないコードや稀なエッジケースを処理するコードを捨てる
ことが大好きです。デバッグの価値を知っており、デバッグの向上に役立つツールを
書くことに時間を費やすことをためらいません。

ユーザ空間のプログラムはMimikerプロジェクトの一部ですが、NetBSDや
[suckless][1]プロジェクトから移植したものにすぎません。私たちはカーネルの
開発に集中しています。そちらのほうが面白いからです。デバイスドライバには
あまり時間を費やしたくないので対象となるプラットフォームのリストは小さく
しています。

このプロジェクトに参加したい場合は[Wiki][2]を読んでください。

## 現状

Mimikerはリアルタイム・オペレーティングシステムです。カーネルはプリエンプ
ティブであり、ミューテックスは優先順位継承をサポートしています。割り込み
コンテキストにおける作業は、ソフト割り込みを使って実行するのではなく、
割り込みスレッドに委譲することで最小限に抑えています。

Mimikerは[QEMU][11]と[Renode][12]の制御の下、[MIPS][15]（32ビット）、
[AArch64][9]、[RISC-V][10]（32ビットと64ビット）の各アーキテクチャで
動作します。

Minikerは次のような素晴らしいデバッグツールを持っています。Pythonで
書かれたgdbスクリプト、Kernel Address Sanitizer、Lock dependency validator、
Kernel Concurrency Sanitizerです。gprofを使ったカーネルプロファイリングも
サポートしています。コードベースのコンパイルには [Clang][19]を使用して
いるため、コードの信頼性を高めるための洗練された動的・静的解析アルゴリズムを
採用することができます。

一般的な同期プリミティブ、すなわち、スピンロック、ミューテックス、条件変数が
提供されています。いずれもセマンティクスはシンプルです。FreeBSDやLinuxの
カーネルによくある、同じことをするけどちょっとだけ違うような多くの
プリミティブを提供するようなことはしていません。

Mimikerのカーネルメモリはワイヤード（スワップ不能）なので、FreeBSDとは違って、
カーネルメモリにアクセスする際に正しいロックを選ぶ心配はありません。物理
メモリ用のバディメモリアロケータ、仮想アドレス空間アロケータ、
論文"[MagazinesとVmem][3]"に基づいたスラブアロケータを使用してます。
これらのメモリアロケータはシンプルでありながら効率的です。

Mimikerのドライバ基盤はFreeBSDの[NewBus][14]と同じ方法でハードウェア
レジスタと割り込みの概念を抽象化しています。ドライバのポータビリティには
特に注意を払っています。PCIやUSBバスに接続されたデバイスを自動検出する
エヌメレーションルーチンもあります。起動時にカーネルコンフィギュレーションを
行うために[flat device tree][13]を使用しています。

仮想ファイルシステムとユーザ仮想アドレス空間の管理は大まかにはFreeBSDの
アイデアに基づいています。これらがFreeBSDやLinuxのカーネル並みに成熟する
にはかなりの量の作業が必要です。

## 私たちの自慢

80を超える[syscalls][4]を提供しているのでNetBSDの[Korn Shell][5]や
[Atto Emacs][6]エディタ、[Lua][7]インタプリタなど、さまざまなオープン
ソースのツールを実行できます。ゲームもできます。

![tetris][8]

Mimikerは次の機能をサポートしてます。

* UNIXファイルI/O -- ファイルライクなオブジェクトにアクセスのためのよく知られたAPI
* プロセス間通信 -- POSIXシグナルとパイプ
* ジョブコントロール -- [Korn Shell][18]をそのまま実行できます
* UNIX認証情報 -- ユーザ、グループ、ファイル・パーミッション
* libterminfo -- Mimikerはフルスクリーン端末アプリケーションを実行できます
* [擬似端末][16] --  [script][17]や端末エミュレータを実行できます

## 足りないもの

今後サポートしたいのは次の機能です。

* マルチコアシステム
* QEMUのVirtIOとvirtプラットフォーム
* 不揮発性ストレージデバイス用のファイルシステム、
* TCP/IPプロトコル。

やるべきことはたくさんあります。ロードマップを参照してください。

[1]: https://suckless.org
[2]: https://github.com/cahirwpz/mimiker/wiki
[3]: https://www.usenix.org/legacy/publications/library/proceedings/usenix01/full_papers/bonwick/bonwick.pdf
[4]: https://github.com/cahirwpz/mimiker/blob/master/sys/kern/syscalls.master
[5]: https://man.netbsd.org/ksh.1
[6]: https://github.com/hughbarney/atto
[7]: https://www.lua.org/docs.html
[8]: https://mimiker.ii.uni.wroc.pl/resources/tetris.gif
[9]: https://www.qemu.org/docs/master/system/target-arm.html
[10]: https://www.qemu.org/docs/master/system/target-riscv.html
[11]: https://www.qemu.org
[12]: https://renode.io
[13]: https://wiki.freebsd.org/FlattenedDeviceTree
[14]: https://nostarch.com/download/samples/freebsd-device-drivers_ch7.pdf
[15]: https://www.qemu.org/docs/master/system/target-mips.html
[16]: https://en.wikipedia.org/wiki/Pseudoterminal
[17]: https://man.netbsd.org/script.1
[18]: https://man.netbsd.org/ksh.1
[19]: https://clang.llvm.org/
