# Анализиатор частот идиом

## Сборка

```bash
git clone git@github.com:deevroman/Lama.git && cd Lama && git switch lab2
cd analyzer && cmake . -B cmake-build-debug && cmake --build ./cmake-build-debug
```

## Запуск примера

```bash
    ./cmake-build-debug/lama_analyzer ../interp/tests/performance/Sort.bc
```

Примеры запуске в Github Actions: https://github.com/deevroman/Lama/actions/workflows/tests_lab_3.yml

<details>
<summary>Результат</summary>

```
Idioms stat: 
cnt=31 DROP
cnt=28 DUP
cnt=21 ELEM
cnt=16 CONST 1
cnt=13 CONST 1	->	ELEM
cnt=11 CONST 0
cnt=11 DROP	->	DUP
cnt=11 DUP	->	CONST 1
cnt=10 DROP	->	DROP
cnt=8 CONST 0	->	ELEM
cnt=7 DUP	->	CONST 0
cnt=7 ELEM	->	DROP
cnt=7 LD_A 0
cnt=5 END
cnt=4 DUP	->	DUP
cnt=4 SEXP 0 2
cnt=3 ARRAY 2
cnt=3 BARRAY 2
cnt=3 BARRAY 2	->	JMP 0x2fa
cnt=3 CALL 351 1
cnt=3 DUP	->	ARRAY 2
cnt=3 ELEM	->	ST_L 0
cnt=3 JMP 0x2fa
cnt=3 LD_L 0
cnt=3 LD_L 3
cnt=3 ST_L 0
cnt=3 ST_L 0	->	DROP
cnt=2 BEGIN 1 0
cnt=2 CALL 43 1
cnt=2 CALL 151 1
cnt=2 DUP	->	TAG 0 2
cnt=2 ELEM	->	CONST 0
cnt=2 ELEM	->	CONST 1
cnt=2 EQ
cnt=2 JMP 0x15e
cnt=2 JMP 0x74
cnt=2 LD_L 1
cnt=2 SEXP 0 2	->	BARRAY 2
cnt=2 TAG 0 2
cnt=1 ARRAY 2	->	CJMPnz 0x118
cnt=1 ARRAY 2	->	CJMPnz 0x27d
cnt=1 ARRAY 2	->	CJMPnz 0xc5
cnt=1 BEGIN 1 1
cnt=1 BEGIN 1 6
cnt=1 BEGIN 2 0
cnt=1 BEGIN 1 0	->	LINE 18
cnt=1 BEGIN 1 0	->	LINE 24
cnt=1 BEGIN 1 1	->	LINE 14
cnt=1 BEGIN 1 6	->	LINE 3
cnt=1 BEGIN 2 0	->	LINE 25
cnt=1 CALL 117 1
cnt=1 CJMPnz 0x118
cnt=1 CJMPnz 0x27d
cnt=1 CJMPnz 0x188
cnt=1 CJMPnz 0x1ac
cnt=1 CJMPnz 0xc5
cnt=1 CJMPnz 0x27d	->	DROP
cnt=1 CJMPnz 0x1ac	->	DROP
cnt=1 CJMPz 0x112
cnt=1 CJMPz 0x258
cnt=1 CJMPz 0x6a
cnt=1 CJMPz 0xbf
cnt=1 CJMPz 0x112	->	DUP
cnt=1 CJMPz 0x258	->	CONST 1
cnt=1 CJMPz 0x6a	->	LD_A 0
cnt=1 CJMPz 0xbf	->	DUP
cnt=1 CONST 10000
cnt=1 CONST 0	->	EQ
cnt=1 CONST 0	->	JMP 0x74
cnt=1 CONST 0	->	LINE 9
cnt=1 CONST 1	->	SUB
cnt=1 CONST 1	->	EQ
cnt=1 CONST 1	->	LINE 6
cnt=1 CONST 10000	->	CALL 43 1
cnt=1 DROP	->	CONST 0
cnt=1 DROP	->	JMP 0x106
cnt=1 DROP	->	JMP 0x150
cnt=1 DROP	->	JMP 0x182
cnt=1 DROP	->	JMP 0x2cb
cnt=1 DROP	->	JMP 0x2de
cnt=1 DROP	->	LD_L 5
cnt=1 DROP	->	LINE 5
cnt=1 DROP	->	LINE 15
cnt=1 DROP	->	LINE 16
cnt=1 DUP	->	DROP
cnt=1 ELEM	->	SEXP 0 2
cnt=1 ELEM	->	DUP
cnt=1 ELEM	->	ST_L 1
cnt=1 ELEM	->	ST_L 2
cnt=1 ELEM	->	ST_L 3
cnt=1 ELEM	->	ST_L 4
cnt=1 ELEM	->	ST_L 5
cnt=1 EQ	->	CJMPz 0x112
cnt=1 EQ	->	CJMPz 0xbf
cnt=1 FAIL 7 17
cnt=1 FAIL 14 9
cnt=1 GT
cnt=1 GT	->	CJMPz 0x258
cnt=1 JMP 0x106
cnt=1 JMP 0x150
cnt=1 JMP 0x182
cnt=1 JMP 0x2cb
cnt=1 JMP 0x2de
cnt=1 LD_A 0	->	CONST 1
cnt=1 LD_A 0	->	DUP
cnt=1 LD_A 0	->	LD_A 0
cnt=1 LD_A 0	->	CJMPz 0x6a
cnt=1 LD_A 0	->	CALL 351 1
cnt=1 LD_A 0	->	CALL 151 1
cnt=1 LD_A 0	->	BARRAY 2
cnt=1 LD_L 2
cnt=1 LD_L 4
cnt=1 LD_L 5
cnt=1 LD_L 0	->	SEXP 0 2
cnt=1 LD_L 0	->	JMP 0x15e
cnt=1 LD_L 0	->	CALL 151 1
cnt=1 LD_L 1	->	GT
cnt=1 LD_L 1	->	LD_L 3
cnt=1 LD_L 2	->	CALL 351 1
cnt=1 LD_L 3	->	LD_L 0
cnt=1 LD_L 3	->	LD_L 1
cnt=1 LD_L 3	->	LD_L 4
cnt=1 LD_L 4	->	SEXP 0 2
cnt=1 LD_L 5	->	LD_L 3
cnt=1 LINE 3
cnt=1 LINE 5
cnt=1 LINE 6
cnt=1 LINE 7
cnt=1 LINE 9
cnt=1 LINE 14
cnt=1 LINE 15
cnt=1 LINE 16
cnt=1 LINE 18
cnt=1 LINE 20
cnt=1 LINE 24
cnt=1 LINE 25
cnt=1 LINE 27
cnt=1 LINE 3	->	LD_A 0
cnt=1 LINE 5	->	LD_L 3
cnt=1 LINE 6	->	LD_L 1
cnt=1 LINE 7	->	LD_L 2
cnt=1 LINE 9	->	LD_A 0
cnt=1 LINE 14	->	LD_A 0
cnt=1 LINE 15	->	LD_L 0
cnt=1 LINE 16	->	LD_L 0
cnt=1 LINE 18	->	LINE 20
cnt=1 LINE 20	->	LD_A 0
cnt=1 LINE 24	->	LD_A 0
cnt=1 LINE 25	->	LINE 27
cnt=1 LINE 27	->	CONST 10000
cnt=1 SEXP 0 2	->	JMP 0x74
cnt=1 SEXP 0 2	->	CALL 351 1
cnt=1 ST_L 1
cnt=1 ST_L 2
cnt=1 ST_L 3
cnt=1 ST_L 4
cnt=1 ST_L 5
cnt=1 ST_L 1	->	DROP
cnt=1 ST_L 2	->	DROP
cnt=1 ST_L 3	->	DROP
cnt=1 ST_L 4	->	DROP
cnt=1 ST_L 5	->	DROP
cnt=1 SUB
cnt=1 SUB	->	CALL 43 1
cnt=1 TAG 0 2	->	CJMPnz 0x188
cnt=1 TAG 0 2	->	CJMPnz 0x1ac
```

</details>