# API и sequencer: техническая основа к EK1

Текущая версия — минимальная запускаемая основа, а не готовый торговый
движок. HTTP API принимает и валидирует команды, а sequencer присваивает каждой
принятой команде единый монотонно возрастающий номер.

HTTP/JSON — прототип технической основы к EK1, а не целевой сетевой протокол
биржи. Целевой бинарный TCP gateway, аутентификация сессий и ограничение частоты
сообщений будут добавляться отдельно; текущий прототип проверяет границу
валидации и последовательную нумерацию команд.

## Граница реализации

Реализовано:

- HTTP-сервер Crow на `127.0.0.1`;
- проверка структуры поддерживаемых команд;
- отклонение некорректной команды до выдачи sequence number;
- потокобезопасная сквозная нумерация принятых команд;
- endpoint состояния sequencer;
- автоматическая проверка последовательности, в том числе при конкурентной
  подаче.

Осознанно оставлено заглушкой:

- применение команды к matching engine;
- проверка аутентификации и прав;
- сохранение команд в WAL;
- восстановление после перезапуска.

Статус `accepted_stub` в ответе явно показывает эту границу. Точка будущей
интеграции WAL находится внутри `CommandSequencer::submit`: после назначения
номера и до передачи команды ядру.

## Сборка и запуск

Требования: CMake 3.20+, Git и компилятор с поддержкой C++20. На Windows
проверена сборка с MSVC 19.41 и MinGW-w64/GCC 13.2, в Ubuntu WSL — с GCC 13.3.
При первой конфигурации CMake скачивает закреплённые версии Crow 1.3.5 и Asio
1.38.2; для последующих сборок используются уже загруженные исходники из
каталога сборки.

```powershell
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
.\build\ome_server.exe
```

CMake использует компилятор и генератор, выбранные в окружении или IDE. Для
многоконфигурационного генератора, например Visual Studio, конфигурацию нужно
указать явно:

```powershell
cmake --build build --config Debug --parallel
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\ome_server.exe
```

В Linux исполняемые файлы не имеют расширения `.exe`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/ome_server
```

По умолчанию сервер слушает только loopback-интерфейс на порту `8080`. Другой
порт можно указать так:

```powershell
.\build\ome_server.exe --port 8090
```

В Linux та же настройка выглядит как `./build/ome_server --port 8090`.

## Простая проверка

Сервер должен продолжать работать в отдельном терминале. В PowerShell каждая
строка ниже является самостоятельной командой и не использует перенос через
обратный апостроф.

### Windows PowerShell

```powershell
Invoke-RestMethod -Uri 'http://127.0.0.1:8080/health'
Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8080/v1/commands' -ContentType 'application/json' -Body (@{ type = 'PLACE_LIMIT'; trader_id = 'trader-1'; side = 'BUY'; price = 100; quantity = 5 } | ConvertTo-Json -Compress)
Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8080/v1/commands' -ContentType 'application/json' -Body (@{ type = 'CANCEL_ORDER'; trader_id = 'trader-1'; order_id = 'order-1' } | ConvertTo-Json -Compress)
try { Invoke-RestMethod -Method Post -Uri 'http://127.0.0.1:8080/v1/commands' -ContentType 'application/json' -Body (@{ type = 'PLACE_LIMIT'; trader_id = 'trader-1'; side = 'BUY'; price = 1.5; quantity = 5 } | ConvertTo-Json -Compress) } catch { $_.ErrorDetails.Message }
Invoke-RestMethod -Uri 'http://127.0.0.1:8080/v1/sequencer'
```

### Linux

```bash
curl -sS http://127.0.0.1:8080/health
curl -sS -X POST http://127.0.0.1:8080/v1/commands -H 'Content-Type: application/json' -d '{"type":"PLACE_LIMIT","trader_id":"trader-1","side":"BUY","price":100,"quantity":5}'
curl -sS -X POST http://127.0.0.1:8080/v1/commands -H 'Content-Type: application/json' -d '{"type":"CANCEL_ORDER","trader_id":"trader-1","order_id":"order-1"}'
curl -sS -o /dev/null -w '%{http_code}\n' -X POST http://127.0.0.1:8080/v1/commands -H 'Content-Type: application/json' -d '{"type":"PLACE_LIMIT","trader_id":"trader-1","side":"BUY","price":1.5,"quantity":5}'
curl -sS http://127.0.0.1:8080/v1/sequencer
```

Ожидается:

- первая принятая команда получает `sequence: 1`;
- вторая — `sequence: 2`;
- обе имеют статус `accepted_stub`;
- `/v1/sequencer` возвращает `last_sequence: 2`;
- команда с дробной, нулевой, отрицательной или не представимой в `int64` ценой
  либо количеством получает HTTP 400 и не расходует номер последовательности.

## Контракт

### `GET /health`

Проверка доступности процесса.

### `GET /v1/sequencer`

Возвращает последний выданный sequence number.

### `POST /v1/commands`

Поддерживаемые типы команд:

- `PLACE_LIMIT`: обязательны `trader_id`, `side` (`BUY`/`SELL`), положительные
  целые `price` и `quantity`;
- `CANCEL_ORDER`: обязательны `trader_id` и `order_id`;
- `OPEN_SESSION`;
- `CLOSE_SESSION`.

Последние две команды пока не имеют проверки административной роли и поэтому
не должны считаться безопасно реализованными. Контроль доступа будет добавлен
до подключения API к реальному ядру.
