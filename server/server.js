require('dotenv').config(); 
const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');

require('./db'); 

const app = express();
const server = http.createServer(app);
const io = new Server(server, { cors: { origin: '*' } });

const PORT = process.env.PORT || 5000;

// ✅ CORS POUR EXPRESS (IMPORTANT)
app.use(cors());

// Middleware JSON
app.use(express.json());

// Injection Socket.IO dans req
app.use((req, res, next) => {
  req.io = io;
  next();
});

// Routes
app.use('/api', require('./api/routes'));

server.listen(PORT, () => {
  console.log(`✅ Server running on port ${PORT}`);
});
