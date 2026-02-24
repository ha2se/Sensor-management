const mongoose = require('mongoose');

const uri = process.env.MONGO_URI;

if (!uri) {
  console.error('❌ MONGO_URI non définie');
  process.exit(1);
}

mongoose.connect(uri, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});

mongoose.connection.on('connected', () => {
  console.log('✅ MongoDB connecté');
});

mongoose.connection.on('error', (err) => {
  console.error('❌ MongoDB erreur:', err);
});
