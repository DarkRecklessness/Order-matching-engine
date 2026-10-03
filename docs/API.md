# API и sequencer: техническая основа к EK1

Текущая версия — минимальная запускаемая основа, а не готовый торговый
движок. HTTP API принимает и валидирует команды, а sequencer присваивает каждой
принятой команде единый монотонно возрастающий номер.

Связанные требования, угрозы и решения приведены в
[концепции безопасности API и sequencer](API_SECURITY.md).

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
проверена сборка с MinGW-w64/GCC 13.2. При первой конфигурации CMake скачивает
закреплённые версии Crow 1.3.5 и Asio 1.38.2; для последующих сборок используются
уже загруженные исходники из каталога сборки.

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

По умолчанию сервер слушает только loopback-интерфейс на порту `8080`. Другой
порт можно указать так:

```powershell
.\build\ome_server.exe --port 8090
```

## Простая проверка

```powershell
Invoke-RestMethod http://127.0.0.1:8080/health

$first = @{
    type = 'PLACE_LIMIT'
    trader_id = 'trader-1'
    side = 'BUY'
    price = 100
    quantity = 5
} | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8080/v1/commands `
    -ContentType 'application/json' -Body $first

$second = @{
    type = 'CANCEL_ORDER'
    trader_id = 'trader-1'
    order_id = 'order-1'
} | ConvertTo-Json
Invoke-RestMethod -Method Post -Uri http://127.0.0.1:8080/v1/commands `
    -ContentType 'application/json' -Body $second

Invoke-RestMethod http://127.0.0.1:8080/v1/sequencer
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
