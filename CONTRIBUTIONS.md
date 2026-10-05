# Вклад участников — Stakan: Order Matching Engine

**Документы проекта:** паспорт — [PROJECT.md](PROJECT.md) · требования безопасности `SR-*` — [SECURITY_REQUIREMENTS.md](SECURITY_REQUIREMENTS.md) · модель угроз `T-*` — [THREAT_MODEL.md](THREAT_MODEL.md) · проектные решения `D-*` — [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md) · вклад участников — [CONTRIBUTIONS.md](CONTRIBUTIONS.md) · использование ИИ — [AI_USAGE.md](AI_USAGE.md)

Требования, угрозы и решения разработаны на общих обсуждениях команды. За каждым участником закреплено одно проектное решение и все цепочки, которые к нему ведут: `M1` — [D-01](DESIGN_DECISIONS.md#d-01), `M2` — [D-03](DESIGN_DECISIONS.md#d-03), `M3` — [D-02](DESIGN_DECISIONS.md#d-02). Участник свёл угрозы, требования, решение и будущие проверки своих цепочек в согласованный вид и отвечает за них.

## M1

Решение [D-01](DESIGN_DECISIONS.md#d-01): заявки адресуются номером участника, сам участник определяется по сессии.

| Результат | Вклад | Проверяемый след |
| --- | --- | --- |
| Минимальная техническая основа: HTTP API и sequencer | Написал HTTP API с проверкой базовой структуры команд и потокобезопасный sequencer, присваивающий принятым командам единую непрерывную нумерацию | [PROJECT.md, §10](PROJECT.md#sec-10) |
| Цепочка [T-01](THREAT_MODEL.md#t-01) → [SR-01](SECURITY_REQUIREMENTS.md#sr-01) → [D-01](DESIGN_DECISIONS.md#d-01): только свои заявки | Ведёт цепочку: согласовал сценарий отмены чужой заявки по публичному ID, критерий неразличимого отказа и адресацию заявок клиентским ID с поиском только среди заявок участника; сверил проверки [D-01](DESIGN_DECISIONS.md#d-01) с критерием [SR-01](SECURITY_REQUIREMENTS.md#sr-01) | [T-01](THREAT_MODEL.md#t-01), [SR-01](SECURITY_REQUIREMENTS.md#sr-01), [D-01](DESIGN_DECISIONS.md#d-01) |
| Цепочки [T-04](THREAT_MODEL.md#t-04) → [SR-03](SECURITY_REQUIREMENTS.md#sr-03) → [D-01](DESIGN_DECISIONS.md#d-01) и [T-05](THREAT_MODEL.md#t-05) → [SR-04](SECURITY_REQUIREMENTS.md#sr-04) → [D-01](DESIGN_DECISIONS.md#d-01): приватность и обезличенность | Ведёт цепочки: адресация отчёта владельцу заявки, отчёт без контрагента, публичное событие без владельца и клиентского ID, сквозной биржевой ID | [T-04](THREAT_MODEL.md#t-04), [T-05](THREAT_MODEL.md#t-05), [SR-03](SECURITY_REQUIREMENTS.md#sr-03), [SR-04](SECURITY_REQUIREMENTS.md#sr-04), [D-01](DESIGN_DECISIONS.md#d-01) |
| Цепочка [SR-02](SECURITY_REQUIREMENTS.md#sr-02) → [D-01](DESIGN_DECISIONS.md#d-01): разделение торговых и административных полномочий | Ведёт цепочку: определение роли при входе и допуск типа команды по роли в gateway с повторной проверкой в ядре | [SR-02](SECURITY_REQUIREMENTS.md#sr-02), [D-01](DESIGN_DECISIONS.md#d-01) |

## M2

Решение [D-03](DESIGN_DECISIONS.md#d-03): ядро исполняет только то, что уже записано на диск.

| Результат | Вклад | Проверяемый след |
| --- | --- | --- |
| Паспорт проекта | Описал назначение и границу продукта, пользователей и их полномочия, семь сквозных сценариев, значимые данные, компоненты и их взаимодействие, инварианты ядра, путь к первой работающей версии и текущую техническую основу | [PROJECT.md](PROJECT.md) |
| Цепочка [T-06](THREAT_MODEL.md#t-06) → [SR-07](SECURITY_REQUIREMENTS.md#sr-07) → [D-03](DESIGN_DECISIONS.md#d-03): сделка не теряется при сбое | Ведёт цепочку: согласовал сценарий потери сделки, критерий сохранности сообщённого клиентам и порядок «sequencer → журнал → ядро»; сверил, что проверки 1–2 [D-03](DESIGN_DECISIONS.md#d-03) (принудительное завершение с остановкой перед записью, многократное падение под нагрузкой) воспроизводимы и подтверждают критерий [SR-07](SECURITY_REQUIREMENTS.md#sr-07) | [T-06](THREAT_MODEL.md#t-06), [SR-07](SECURITY_REQUIREMENTS.md#sr-07), [D-03](DESIGN_DECISIONS.md#d-03) |
| Цепочка [T-07](THREAT_MODEL.md#t-07) → [SR-08](SECURITY_REQUIREMENTS.md#sr-08) → [D-03](DESIGN_DECISIONS.md#d-03): целостность журнала | Ведёт цепочку: согласовал сценарий подмены журнала, критерий обнаружения порчи и цепочку хешей с проверкой при восстановлении; зафиксировал остаточный риск полной перезаписи; сверил, что проверка 3 [D-03](DESIGN_DECISIONS.md#d-03) (порча копии журнала) подтверждает критерий [SR-08](SECURITY_REQUIREMENTS.md#sr-08) | [T-07](THREAT_MODEL.md#t-07), [SR-08](SECURITY_REQUIREMENTS.md#sr-08), [D-03](DESIGN_DECISIONS.md#d-03) |
| Сравнение вариантов в [D-03](DESIGN_DECISIONS.md#d-03) | Сравнил запись до исполнения с исполнением параллельно записи и задержкой исходящих сообщений по задержке, числу контролируемых путей и поведению при остановке диска; сформулировал допущение выбора, условие пересмотра и проверку 1, учитывающую ограничение теста с завершением процесса | [D-03](DESIGN_DECISIONS.md#d-03) |

## M3

Решение [D-02](DESIGN_DECISIONS.md#d-02): формат сообщений проверяет gateway, лимиты — ядро.

| Результат | Вклад | Проверяемый след |
| --- | --- | --- |
| Цепочка [T-02](THREAT_MODEL.md#t-02) → [SR-05](SECURITY_REQUIREMENTS.md#sr-05) → [D-02](DESIGN_DECISIONS.md#d-02): устойчивость ядра | Ведёт цепочку: согласовал сценарий сбоя ядра, повторяющегося при восстановлении, и разделение проверок между gateway и ядром; составил проверки фаззингом и повторным исполнением журнала ([D-02](DESIGN_DECISIONS.md#d-02), 1–2) | [T-02](THREAT_MODEL.md#t-02), [SR-05](SECURITY_REQUIREMENTS.md#sr-05), [D-02](DESIGN_DECISIONS.md#d-02) |
| Цепочка [T-03](THREAT_MODEL.md#t-03) → [SR-09](SECURITY_REQUIREMENTS.md#sr-09) → [D-02](DESIGN_DECISIONS.md#d-02): перегрузка одним клиентом | Ведёт цепочку: лимит частоты и проверки без состояния в gateway до упорядочивания | [T-03](THREAT_MODEL.md#t-03), [SR-09](SECURITY_REQUIREMENTS.md#sr-09), [D-02](DESIGN_DECISIONS.md#d-02) |
| Цепочка [SR-06](SECURITY_REQUIREMENTS.md#sr-06) → [D-02](DESIGN_DECISIONS.md#d-02): пред-торговые лимиты | Ведёт цепочку: единая функция допуска в ядре для `NEW_ORDER` и `REPLACE`, статический ценовой коридор, граница действия `SET_LIMITS` по порядковому номеру | [SR-06](SECURITY_REQUIREMENTS.md#sr-06), [D-02](DESIGN_DECISIONS.md#d-02) |
