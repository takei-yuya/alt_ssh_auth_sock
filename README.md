# LD_PRELOADを使ってSSH_AUTH_SOCK環境変数のソケットが既に切断されてた場合に、別の環境変数からグロブパターン受け取って別のソケットを試すようにするやつ

代替SSH_AUTH_SOCK、ということで libaltsshauthsock.so(仮称)。

## これはなに？

ssh越しにscreenを張った場合、デタッチ/アタッチによって、大元のsshの接続がすでに切断され、環境変数のSSH_AUTH_SOCKが陳腐化してしまう問題、
いわゆるSSH Agent on Screen 問題を、LD_PRELOADの仕組みを作って解決しようとするもの。

仕組みはすんごく単純で、LD_PRELOADで `getenv(2)` をフックし、 `"SSH_AUTH_SOCK"` の生存をチェックして、
死んでいる場合には別の生きているだろうと思われるソケットのパスに変えてしまうだけ。

接続の確認にconnectを試みるので結構オーバーヘッドが大きそう。この辺TODO

## 使い方

1. `libaltsshauthsock.so` をLD_PRELOADに通す

    ```console
    $ echo 'export LD_PRELOAD="/path/to/libaltsshauthsock.so"' >> ~/.bashrc
    ```

2. ソケットのパスパターンを `ALT_SSH_AUTH_SOCK` 環境変数にセットする

    ```console
    $ echo 'export ALT_SSH_AUTH_SOCK="/tmp/ssh-*/agent.*"' >> ~/.bashrc
    ```

3. screenを起動する

    ```console
    $ screen
    ```

あとはいつもの通りにscreenを使うだけ。


## TODO

- Macに対応する。
    - .dylibを作る
    - MacだとLD_PRELOADではなく、DYLD_INSERT_LIBRARIES。
    - NOTE: DYLD_FORCE_FLAT_NAMESPACEの設定が必要らしいので注意。
