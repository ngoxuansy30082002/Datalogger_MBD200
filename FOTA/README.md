# Firmware Management Server

A production-ready full-stack web application for managing firmware (.hex files) for embedded devices with MQTT integration, real-time dashboard, and role-based access control.

## 🚀 Quick Start

### Prerequisites
- Node.js 22+
- MongoDB 7+
- MQTT Broker (Mosquitto) on port 27000

### Development Setup

```bash
# 1. Install backend dependencies
cd backend
npm install

# 2. Install frontend dependencies
cd ../frontend
npm install

# 3. Start MongoDB and MQTT broker (if not running)
# MongoDB default port: 27017
# MQTT broker port: 27000

# 4. Start backend (from backend/)
cd ../backend
npm run dev

# 5. Start frontend (from frontend/)
cd ../frontend
npm run dev
```

Frontend: http://localhost:3000
Backend API: http://localhost:80
Swagger UI: http://localhost:80/api-docs

### Docker Deployment

```bash
docker-compose up -d --build
```

## 🔐 Default Credentials

| Username | Password | Role  |
|----------|----------|-------|
| admin    | admin123 | Admin |

## 📡 API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST   | /api/auth/login | User login |
| POST   | /api/auth/register | Register user (admin) |
| POST   | /api/auth/refresh | Refresh token |
| GET    | /api/firmware | List firmware |
| POST   | /api/firmware/upload | Upload firmware |
| GET    | /api/firmware/download | Download by query |
| GET    | /api/firmware/download/:id | Download by ID |
| GET    | /api/firmware/latest | Check for updates |
| PATCH  | /api/firmware/:id/latest | Mark as latest |
| DELETE | /api/firmware/:id | Delete firmware |
| GET    | /api/devices | List devices |
| GET    | /api/dashboard/stats | Dashboard stats |
| POST   | /api/mqtt/notify | Publish notification |
| GET    | /api/mqtt/status | MQTT status |

## 📂 Project Structure

```
FOTA/
├── docker-compose.yml
├── .env
├── backend/         # Node.js + Express + TypeScript
│   ├── src/
│   │   ├── config/      # DB, MQTT, Swagger config
│   │   ├── models/      # Mongoose schemas
│   │   ├── repositories/# Data access layer
│   │   ├── services/    # Business logic
│   │   ├── controllers/ # Route handlers
│   │   ├── middleware/  # Auth, validation, upload
│   │   ├── routes/      # API routes
│   │   └── utils/       # Helpers
│   └── storage/         # Firmware file storage
├── frontend/        # React + Vite + MUI
│   └── src/
│       ├── api/         # Axios API layer
│       ├── components/  # Reusable components
│       ├── contexts/    # Auth & Socket providers
│       └── pages/       # Page components
└── mosquitto/       # MQTT broker config
```

## 🔌 MQTT Topics

| Direction | Topic | Description |
|-----------|-------|-------------|
| Subscribe | datalogger/+/firmware/query | Firmware update query |
| Subscribe | datalogger/+/status | Device status |
| Subscribe | datalogger/+/heartbeat | Device heartbeat |
| Publish   | datalogger/firmware/notify | Update notification |
| Publish   | datalogger/{id}/response | Query response |
