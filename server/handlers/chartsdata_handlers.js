const SensorData = require("../models/temperature");

exports.getChartData = async (req, res) => {
  try {
     console.log("Params:", req.params);
    console.log("Query:", req.query);
    const { deviceId } = req.params;
    const { period } = req.query;

    if (!deviceId) {
      return res.status(400).json({ error: "deviceId is required" });
    }

    let filter = { deviceId: deviceId };

    if (period && period !== "all") {
      let startDate;

      switch (period) {
        case "24h":
          startDate = new Date(Date.now() - 24 * 60 * 60 * 1000);
          break;

        case "7d":
          startDate = new Date(Date.now() - 7 * 24 * 60 * 60 * 1000);
          break;

        case "30d":
          startDate = new Date(Date.now() - 30 * 24 * 60 * 60 * 1000);
          break;

        default:
          return res.status(400).json({ error: "Invalid period value" });
      }

      // ✅ On filtre sur createdAt
      filter.createdAt = { $gte: startDate };
    }

    const data = await SensorData
      .find(filter)
      .sort({ createdAt: 1 });

    // Convertir en format sérialisable
    const formattedData = data.map(item => ({
      ...item.toObject(),
      createdAt: item.createdAt.toISOString()
    }));

    res.json(formattedData);

  } catch (error) {
    console.error("Chart data error:", error);
    res.status(500).json({ error: "Server error" });
  }
   

};
