module.exports = (io) => {
  io.on('connection', (socket) => {
    console.log('🔌 Client connecté');

    socket.on('disconnect', () => {
      console.log('❌ Client déconnecté');
    });
  });
};
