#!/usr/bin/env bash
# Host-specific deployment helper; run on the test host as root.
set -euo pipefail
if [[ $EUID != 0 || $# != 2 ]]; then
    echo "Usage: sudo $0 /absolute/path/to/library /absolute/path/to/server" >&2
    exit 2
fi
bridge_source=$1
server_dir=$2
[[ -f "$bridge_source" && -d "$server_dir" && $server_dir = /* ]] || exit 2
service_name=satisfactory.service
bridge_target="$server_dir/libsatis_task_bridge.so"
backup_dir="/home/satisfactory/bridge-backups/$(date -u +%Y%m%dT%H%M%SZ)"
install -d -m 0700 "$backup_dir"
systemctl stop "$service_name"
# Never overwrite a library still mapped by a running process.
if [[ -f "$bridge_target" ]]; then cp -p "$bridge_target" "$backup_dir/"; fi
cp -a /etc/systemd/system/satisfactory.service.d "$backup_dir/systemd-dropins"
tar -czf "$backup_dir/saved.tar.gz" -C /home/satisfactory/.config/Epic/FactoryGame Saved
tar -tzf "$backup_dir/saved.tar.gz" >/dev/null
install -o satisfactory -g satisfactory -m 0755 "$bridge_source" "$bridge_target.new"
mv "$bridge_target.new" "$bridge_target"
sha256sum "$bridge_target"
systemctl start "$service_name"
echo "Backup directory: $backup_dir"
systemctl show "$service_name" -p ActiveState -p MainPID -p NRestarts
