export function connect(): [WebSocket, () => void] {
  const socket = new WebSocket("wss://localhost:8870");

  socket.addEventListener("message", (event) => {
    console.log("WS-Message:", event.data);
  });

  return [socket, () => socket.close()];
}
