import React, { createContext, useContext, useEffect, useState, useRef, ReactNode, useCallback } from 'react';
import { io, Socket } from 'socket.io-client';
import { useAuth } from './AuthContext';
import { MqttMessage } from '../types';

interface SocketContextType {
  socket: Socket | null;
  isConnected: boolean;
  mqttMessages: MqttMessage[];
  clearMessages: () => void;
}

const SocketContext = createContext<SocketContextType>({
  socket: null,
  isConnected: false,
  mqttMessages: [],
  clearMessages: () => {},
});

export const useSocket = () => useContext(SocketContext);

export const SocketProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const { isAuthenticated } = useAuth();
  const [socket, setSocket] = useState<Socket | null>(null);
  const [isConnected, setIsConnected] = useState(false);
  const [mqttMessages, setMqttMessages] = useState<MqttMessage[]>([]);
  const socketRef = useRef<Socket | null>(null);

  const clearMessages = useCallback(() => {
    setMqttMessages([]);
  }, []);

  useEffect(() => {
    if (!isAuthenticated) {
      if (socketRef.current) {
        socketRef.current.disconnect();
        socketRef.current = null;
        setSocket(null);
        setIsConnected(false);
      }
      return;
    }

    const token = localStorage.getItem('accessToken');
    if (!token) return;

    const newSocket = io(window.location.origin, {
      auth: { token },
      transports: ['websocket', 'polling'],
      reconnection: true,
      reconnectionDelay: 1000,
      reconnectionAttempts: 10,
    });

    newSocket.on('connect', () => {
      setIsConnected(true);
    });

    newSocket.on('disconnect', () => {
      setIsConnected(false);
    });

    newSocket.on('mqtt:message', (message: MqttMessage) => {
      setMqttMessages((prev) => [message, ...prev].slice(0, 500));
    });

    socketRef.current = newSocket;
    setSocket(newSocket);

    return () => {
      newSocket.disconnect();
    };
  }, [isAuthenticated]);

  return (
    <SocketContext.Provider value={{ socket, isConnected, mqttMessages, clearMessages }}>
      {children}
    </SocketContext.Provider>
  );
};
