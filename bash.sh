./netdiag show interfaces   # список интерфейсов
./netdiag show routes       # таблица маршрутизации
./netdiag show vlans        # VLAN
./netdiag check link eth0   # состояние интерфейса eth0
./netdiag check gateway 192.168.1.1  # проверка шлюза
./netdiag collect           # сбор диагностики в /tmp/netdiag_*.tar.gz