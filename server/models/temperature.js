const mongoose = require('mongoose');

const sensorData = new mongoose.Schema({
    deviceId: {
  type: String,
  required: true
    },
    moisture: {
        type: Number,
        required: true
    },
    temperature: {
        type: Number,
        required: true
    },
        ph: {
        type: Number,
        required: true
    },
    ec: {
        type: Number,
        required: true
    }
}, { timestamps: true });

module.exports = mongoose.model('SensorData', sensorData);
