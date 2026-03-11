# Сборка

```bash
git clone git@github.com:deevroman/Lama.git && cd Lama && git switch lab2
cd interp && make
```

Работоспособоность сборки и тестов можно увидеть в GitHub Action: https://github.com/deevroman/Lama/actions

Также поддерживается сборка с помощью cmake. 
Раскомментировав строки в `CMakeLists.txt` можно включить санитайзеры, -O3, или отладочные логи.

# Тесты

Перед запуском тестов необходимо запустить make в директориях runtime и src. 
Однако для этого потребуется настроить окружение с ocaml. Можно использовать мой docker-образ:

```bash
docker run --platform=linux/amd64 --rm -v $PWD:/lama -it -w /lama trickyfoxy/lama bash 
```

После этого можно запустить сборку

```bash
eval $(opam env --switch=lama --set-switch)
cd ../runtime && make && cd -
cd ../src && make && cd -
```

И наконец запустить регрессионные тесты:
```bash
./run-regression-tests.sh
```

Или тест проиводительности:

```bash
./run-performance-test.sh
```

При запуске в GitHub Actions рекурсивный интерпретатор выполняет Sort.lama за ~9m. 
Реализованный интеративный интерпретатор за ~2.5m. С включенной верификацией за ~2m.

```
./run-performance-test.sh
Sort.lama
Running Lama interpreter...

real	9m19.559s
user	9m18.618s
sys	0m0.881s

Running bytecode interpreter...

real	2m32.642s
user	2m27.699s
sys	0m4.687s

Running bytecode interpreter with verifier...

real	2m0.685s
user	1m54.101s
sys	0m3.327s

```