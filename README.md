# CG-Project3 圖學導論project 3 Trains and Roller Coasters

## 2020/11/5

目前問題

1. 第一人稱視角中，只要兩點之間的orient向量不同，在經過點的時候，會發生跳動

2. 鐵軌中間會有小縫縫(已解決)，把qt0往後挪一點點就好

3. 火車畫不出來，想用vertices來做模型，然後再transform到目前的位置上

## 2020/11/2

現在train可以沿著軌道跑

## 2020/11/2

開始動工
完成linear、cardinal、B spline三種track的產生
用switch case在spline type上會有奇怪的問題發生
看來是少用switch為妙
