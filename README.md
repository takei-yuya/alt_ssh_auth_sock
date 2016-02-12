# LD_PRELOADを使ってSSH_AUTH_SOCK環境変数のファイルが既に消されてた場合に、別の環境変数からグロブパターン受け取って別のソケットを試すようにするやつ

代替SSH_AUTH_SOCK、ということで libaltsshauthsock.so(仮称)。

## これはなに？

ssh越しにscreenを張った場合、デタッチ/アタッチによって、大元のsshの接続がすでに切断され、環境変数のSSH_AUTH_SOCKが陳腐化してしまう問題、
いわゆるSSH Agent on Screen 問題を、LD_PRELOADの仕組みを作って解決しようとするもの。
仕組みはすんごく単純で、LD_PRELOADで `getrnv(2)` をフックし、 `"SSH_AUTH_SOCK"` の存在をチェックして、
死んでいる場合には別の生きているだろうと思われるソケットのパスに変えてしまうだけ。
実際には、ソケットは存在しているけどセッションは既に切れている、とかあるっぽいので、完全とは言えないけどだいたい上手くいく。

## 使い方

1. `libaltsshauthsock.so` をLD_PRELOADに通す

    ```console
    $ export LD_PRELOAD="/path/to/libaltsshauthsock.so"
    ```

2. ソケットのパスパターンを `ALT_SSH_AUTH_SOCK` 環境変数にセットする

    ```console
    $ export ALT_SSH_AUTH_SOCK="/tmp/ssh-*/agent.*"
    ```

3. screenを起動する

    ```console
    $ screen
    ```

あとはいつもの通りにscreenを使うだけ。
screen上のセッションで、SSH_AUTH_SOCK環境変数を参照するプロセスが動くたび、SSH_AUTH_SOCKの存在をチェックし、
もし存在しない場合には、別のALT_SSH_AUTH_SOCKのパターンにマッチするファイルのパスにすり替えてくれる。


## TODO

MacのDYLD_INSERT_LIBRARIESに対応する。
NOTE: DYLD_FORCE_FLAT_NAMESPACEの設定が必要らしいので注意。
