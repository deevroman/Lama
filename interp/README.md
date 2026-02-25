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

При запуске в GitHub Actions рекурсивный интерпретатор выполняет Sort.lama за ~9s. 
Реализованный интеративный интерпретатор за ~3s.

https://github.com/deevroman/Lama/actions/runs/22417727064/job/64907573924
```
./run-performance-test.sh
Sort.lama
Running Lama interpreter...

real	9m11.526s
user	9m10.248s
sys	0m1.214s

Running bytecode interpreter...

real	2m58.768s
user	2m53.716s
sys	0m4.137s
```