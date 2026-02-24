const express =  require('express');
const router = express.Router();
const sensorData= require('../models/temperature');
const chartsDataHandlers = require('../handlers/chartsdata_handlers.js');

router.get('/charts/:deviceId', chartsDataHandlers.getChartData);
  
router.post('/data', async (req, res) => {
    console.log('📩 POST /api/data appelé');

    try {
        const { moisture, temperature, ph, ec } = req.body;

        if (moisture === undefined || temperature === undefined || ph === undefined || ec  === undefined) {
            return res.status(400).json({ error: 'Données invalides' });
        }

        const data = await sensorData.create({
            deviceId: req.body.deviceId || 'unknown',
            moisture,
            temperature,
            ph,
            ec
        });

        console.log('Données sauvegardées :', data);

        req.io.emit('sensorData', {
            deviceId: data.deviceId,
            moisture: data.moisture,
            temperature: data.temperature,
            ph : data.ph,
            ec : data.ec,
            createdAt: data.createdAt.toISOString()
        });

        res.sendStatus(200);

    } catch (error) {
        console.error('Erreur sauvegarde :', error);
        res.status(500).json({ error: 'Erreur serveur' });
    }
});
router.get('/sensor-data', async (req, res) => {
    try {
        const data = await sensorData.find().sort({ createdAt: 1 });

        res.json(data.map(item => ({
            deviceId: item.deviceId,
            moisture: item.moisture,
            temperature: item.temperature,
            ph: item.ph,
            ec: item.ec,
            createdAt: item.createdAt.toISOString()
        })));

    } catch (error) {
        console.error('Erreur récupération données :', error);
        res.status(500).json({ error: 'Erreur serveur' });
    }
});
module.exports = router;
