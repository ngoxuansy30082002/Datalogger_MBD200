# Deployment Guide

## Project Overview

The Firmware Management Server is a full-stack web application for managing firmware files (.hex) for embedded devices (Datalogger MBD200) with MQTT-based over-the-air (OTA) update support.

### Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Frontend   │────▶│   Backend   │────▶│   MongoDB   │
│  (Nginx:3000)│     │ (Node.js:80)│     │  (:27017)   │
└─────────────┘     └──────┬──────┘     └─────────────┘
                           │
                    ┌──────▼──────┐
                    │ MQTT Broker │
                    │  (External) │
                    └─────────────┘
```

- **Frontend**: React SPA served by Nginx, proxies API/WebSocket to backend
- **Backend**: Node.js/Express API server with JWT auth, firmware management, MQTT client
- **MongoDB**: Stores firmware metadata, devices, users, download logs
- **MQTT**: Connects to an external broker (e.g., EMQX) for device communication

### Port Mapping

| Service  | Internal Port | External Port | Purpose                    |
|----------|--------------|---------------|----------------------------|
| Frontend | 3000         | 3000          | Web UI + API proxy         |
| Backend  | 80           | 80            | REST API + firmware download |
| MongoDB  | 27017        | —             | Internal only              |

> **Note**: Embedded devices access firmware downloads directly via the backend on port `80`. The frontend on port `3000` is for the web management UI.

---

## Project Structure

```
FOTA/
├── docker-compose.yml          # Docker Compose orchestration
├── .env                        # Environment variables (not committed)
├── .env.example                # Environment template
├── docs/
│   └── DEPLOYMENT.md           # This file
├── backend/
│   ├── Dockerfile              # Multi-stage Node.js build
│   ├── .dockerignore
│   ├── package.json
│   ├── tsconfig.json
│   └── src/
│       ├── index.ts            # Entry point
│       ├── app.ts              # Express app setup
│       ├── config/             # Environment, DB, MQTT, Swagger config
│       ├── controllers/        # Request handlers
│       ├── middleware/          # Auth, validation, upload, error handling
│       ├── models/             # Mongoose schemas
│       ├── repositories/       # Database access layer
│       ├── routes/             # API route definitions
│       ├── services/           # Business logic
│       ├── types/              # TypeScript type definitions
│       └── utils/              # Helper utilities
└── frontend/
    ├── Dockerfile              # Multi-stage Vite + Nginx build
    ├── .dockerignore
    ├── nginx.conf              # Nginx reverse proxy config
    ├── package.json
    └── src/
        ├── App.tsx             # Main React component
        ├── api/                # Axios API client
        ├── components/         # Reusable UI components
        ├── contexts/           # React contexts (Auth, Socket)
        ├── pages/              # Page components
        ├── types/              # TypeScript types
        └── theme.ts            # MUI theme configuration
```

---

## Environment Configuration

### Files

| File           | Purpose                                    | Committed? |
|----------------|---------------------------------------------|------------|
| `.env.example` | Template with all variables and descriptions | ✅ Yes     |
| `.env`         | Actual configuration with secrets            | ❌ No      |

### Environment Variables

| Variable             | Required | Default                       | Description                                           |
|----------------------|----------|-------------------------------|-------------------------------------------------------|
| `NODE_ENV`           | No       | `development`                 | Environment mode (`development` or `production`)       |
| `PORT`               | No       | `80`                          | Backend HTTP server port                               |
| `SERVER_BASE_URL`    | **Yes**  | `http://localhost`            | Full public URL of the backend (used in MQTT firmware download URLs). Example: `http://20.243.20.10` |
| `MONGO_URI`          | No       | `mongodb://localhost:27017/firmware_manager` | MongoDB connection string. Use `mongodb://mongodb:27017/firmware_manager` in Docker |
| `MQTT_BROKER_URL`    | No       | `mqtt://broker.emqx.io:1883` | MQTT broker connection URL                             |
| `JWT_SECRET`         | **Yes**  | —                             | Secret key for JWT access tokens. **Change in production!** |
| `JWT_REFRESH_SECRET` | **Yes**  | —                             | Secret key for JWT refresh tokens. **Change in production!** |
| `JWT_EXPIRY`         | No       | `1h`                          | Access token expiration duration                       |
| `JWT_REFRESH_EXPIRY` | No       | `7d`                          | Refresh token expiration duration                      |
| `STORAGE_PATH`       | No       | `./storage`                   | Path for uploaded firmware files. Use `/app/storage` in Docker |
| `LOG_PATH`           | No       | `./logs`                      | Path for application log files. Use `/app/logs` in Docker |
| `CERT_PATH`          | No       | `../certs`                    | Path for SSL certificates (optional)                   |
| `HTTPS_PORT`         | No       | `443`                         | HTTPS port (only used if SSL certificates exist)       |
| `MAX_FILE_SIZE`      | No       | `52428800`                    | Maximum firmware file upload size in bytes (50MB)       |
| `ADMIN_USERNAME`     | No       | `admin`                       | Default admin username (created on first run)          |
| `ADMIN_PASSWORD`     | **Yes**  | —                             | Default admin password. **Change in production!**      |
| `CORS_ORIGIN`        | No       | `*`                           | Allowed CORS origins (comma-separated or `*`)          |

---

## Deployment Steps

### 1. Clone the Repository

```bash
git clone <repository-url>
cd FOTA
```

### 2. Configure Environment

```bash
cp .env.example .env
```

Edit `.env` with your production values:

```bash
nano .env
```

**Minimum required changes:**

```env
SERVER_BASE_URL=http://YOUR_SERVER_IP
JWT_SECRET=your-strong-random-secret-key-here
JWT_REFRESH_SECRET=another-strong-random-secret-key-here
ADMIN_PASSWORD=your-secure-admin-password
```

> **Tip**: Generate secure secrets with: `openssl rand -hex 32`

### 3. Build the Project

```bash
docker compose build
```

This builds both the backend and frontend Docker images using multi-stage builds.

### 4. Start the Services

```bash
docker compose up -d
```

### 5. Verify the Deployment

Check that all containers are running:

```bash
docker compose ps
```

Expected output:

```
NAME           STATUS                   PORTS
fw-backend     Up (healthy)             0.0.0.0:80->80/tcp
fw-frontend    Up                       0.0.0.0:3000->3000/tcp
fw-mongodb     Up (healthy)             27017/tcp
```

Check backend health:

```bash
curl http://localhost:80/health
```

Expected response:

```json
{"status":"ok","timestamp":"2026-06-26T12:00:00.000Z"}
```

View logs:

```bash
docker compose logs -f
```

### 6. Access the Application

| URL                               | Purpose                  |
|------------------------------------|--------------------------|
| `http://YOUR_SERVER_IP:3000`       | Web Management UI        |
| `http://YOUR_SERVER_IP/api-docs`   | Swagger API Documentation|
| `http://YOUR_SERVER_IP/health`     | Health Check Endpoint    |

Default login credentials (change after first login):
- **Username**: Value of `ADMIN_USERNAME` (default: `admin`)
- **Password**: Value of `ADMIN_PASSWORD`

---

## Updating the Application

To deploy a new version:

```bash
# Pull latest code
git pull

# Rebuild images
docker compose build

# Restart services (zero-downtime)
docker compose up -d
```

If only the backend changed:

```bash
docker compose build backend
docker compose up -d backend
```

If only the frontend changed:

```bash
docker compose build frontend
docker compose up -d frontend
```

---

## Backup

### MongoDB

```bash
# Create backup
docker compose exec mongodb mongodump --out /data/backup

# Copy backup to host
docker cp fw-mongodb:/data/backup ./backup-$(date +%Y%m%d)
```

Or using `mongodump` directly from host:

```bash
docker compose exec mongodb mongodump \
  --db firmware_manager \
  --archive=/data/backup.gz \
  --gzip

docker cp fw-mongodb:/data/backup.gz ./mongodb-backup-$(date +%Y%m%d).gz
```

### Firmware Storage

```bash
# Copy firmware files from Docker volume
docker cp fw-backend:/app/storage ./storage-backup-$(date +%Y%m%d)
```

### Logs

```bash
# Copy log files from Docker volume
docker cp fw-backend:/app/logs ./logs-backup-$(date +%Y%m%d)
```

### Full Backup Script

```bash
#!/bin/bash
BACKUP_DIR="./backups/$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"

# MongoDB
docker compose exec -T mongodb mongodump --db firmware_manager --archive --gzip > "$BACKUP_DIR/mongodb.gz"

# Firmware storage
docker cp fw-backend:/app/storage "$BACKUP_DIR/storage"

# Logs
docker cp fw-backend:/app/logs "$BACKUP_DIR/logs"

echo "Backup completed: $BACKUP_DIR"
```

---

## Restore

### MongoDB

```bash
# Stop backend to prevent writes during restore
docker compose stop backend

# Restore from archive
cat ./mongodb-backup.gz | docker compose exec -T mongodb mongorestore \
  --db firmware_manager \
  --archive \
  --gzip \
  --drop

# Start backend
docker compose start backend
```

### Firmware Storage

```bash
# Copy firmware files back into container
docker cp ./storage-backup/. fw-backend:/app/storage/
```

### Full Restore

```bash
# Stop services
docker compose stop backend

# Restore MongoDB
cat ./backups/YYYYMMDD/mongodb.gz | docker compose exec -T mongodb mongorestore \
  --db firmware_manager --archive --gzip --drop

# Restore firmware storage
docker cp ./backups/YYYYMMDD/storage/. fw-backend:/app/storage/

# Restart
docker compose start backend
```

---

## Troubleshooting

### Backend Cannot Start

**Symptom**: `fw-backend` container exits immediately or restart loops.

```bash
docker compose logs backend
```

**Common causes**:
- Missing or invalid environment variables → Check `.env` file
- MongoDB not ready → Ensure `depends_on` and healthcheck are configured
- Port 80 already in use → Stop conflicting services or change `PORT`

### Frontend Cannot Connect to Backend

**Symptom**: API calls return 502 or connection refused in the browser.

```bash
docker compose logs frontend
```

**Common causes**:
- Backend not healthy yet → Wait for backend healthcheck to pass
- Nginx config error → Check `frontend/nginx.conf` for correct backend hostname (`backend:80`)
- Network issue → Verify both containers are on `fw-network`: `docker network inspect fw-network`

### MongoDB Connection Failed

**Symptom**: Backend logs show `MongoDB connection failed`.

```bash
docker compose logs mongodb
```

**Common causes**:
- MongoDB container not running → `docker compose up -d mongodb`
- Wrong `MONGO_URI` → Must use `mongodb://mongodb:27017/firmware_manager` (Docker service name, not `localhost`)
- Volume permissions → `docker compose down -v` and restart (⚠️ destroys data)

### MQTT Connection Failed

**Symptom**: Backend logs show `MQTT error` or no MQTT subscription messages.

**Common causes**:
- External broker unreachable → Verify network connectivity: `docker compose exec backend wget -q -O- http://broker.emqx.io`
- Wrong broker URL → Check `MQTT_BROKER_URL` in `.env`
- Firewall blocking port 1883 → Open outbound port 1883

### Firmware Upload Failed

**Symptom**: Upload returns 500 error or file not saved.

**Common causes**:
- File too large → Increase `MAX_FILE_SIZE` and `client_max_body_size` in nginx.conf
- Storage volume not mounted → Check `docker compose ps` and volume mounts
- Permission denied → Container user cannot write to `/app/storage`

### Firmware Download Failed

**Symptom**: Download returns 404 or empty file.

**Common causes**:
- File missing from storage → Container was recreated without volume persistence
- Wrong `SERVER_BASE_URL` → Devices use this URL to download; ensure it's reachable
- Nginx timeout → Large files may timeout; check `proxy_read_timeout` in nginx.conf

### Docker Container Restart Loop

**Symptom**: Container status shows `Restarting`.

```bash
docker compose logs --tail=50 <service-name>
```

**Common causes**:
- Application crash on startup → Check logs for error details
- Healthcheck failing → Increase `start_period` in healthcheck config
- Out of memory → Increase Docker memory limits

---

## Useful Docker Commands

```bash
# View all container statuses
docker compose ps

# View logs (all services)
docker compose logs

# View logs (specific service, follow mode)
docker compose logs -f backend

# Restart a specific service
docker compose restart backend
docker compose restart frontend

# Stop all services
docker compose down

# Stop and remove volumes (⚠️ DESTROYS ALL DATA)
docker compose down -v

# Start all services
docker compose up -d

# Rebuild and restart a specific service
docker compose up -d --build backend

# Enter a container shell
docker compose exec backend sh
docker compose exec mongodb mongosh

# Check resource usage
docker stats

# Clean up unused images
docker image prune

# List Docker volumes
docker volume ls

# Inspect a volume
docker volume inspect fota_mongo_data
```
