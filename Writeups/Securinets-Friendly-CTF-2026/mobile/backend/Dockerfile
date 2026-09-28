# Mobile-track backend: app.py (:8000 HTTP, :8443 HTTPS) and
# rogue_sdk_collector.py (:9090), the three services start_all.sh runs
# locally. One image, one container, all three processes -- same pattern
# malware-track/quiz uses for its two ports.
#
# Flags are NEVER baked in. secrets/flags.json is gitignored and must be
# bind-mounted read-only at runtime (see docker-compose.yml); CTF_FLAGS
# points app.py and rogue_sdk_collector.py at it. debriefs.json and the
# certs/ pair ARE tracked in git and safe to bake in -- see backend/README.md
# and docs/PROGRESS.md for why.
FROM python:3.12-slim

ENV PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1

RUN useradd --system --no-create-home --shell /usr/sbin/nologin backend

WORKDIR /srv/backend

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY app.py rogue_sdk_collector.py debriefs.json docker-entrypoint.sh ./
COPY --chown=backend:backend certs/ ./certs/

RUN chmod +x docker-entrypoint.sh

USER backend

EXPOSE 8000 8443 9090

ENTRYPOINT ["./docker-entrypoint.sh"]
