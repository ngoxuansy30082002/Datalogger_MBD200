import React, { useState } from 'react';
import {
  Box, Card, CardContent, TextField, Button, Typography, InputAdornment,
  IconButton, CircularProgress, alpha, keyframes,
} from '@mui/material';
import { Visibility, VisibilityOff, Login as LoginIcon } from '@mui/icons-material';
import { useAuth } from '../contexts/AuthContext';
import { useNavigate } from 'react-router-dom';
import { useSnackbar } from 'notistack';

const float = keyframes`
  0%, 100% { transform: translateY(0px) rotate(0deg); }
  50% { transform: translateY(-20px) rotate(5deg); }
`;

const pulse = keyframes`
  0%, 100% { opacity: 0.3; }
  50% { opacity: 0.6; }
`;

const LoginPage: React.FC = () => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [loading, setLoading] = useState(false);
  const { login } = useAuth();
  const navigate = useNavigate();
  const { enqueueSnackbar } = useSnackbar();

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!username || !password) {
      enqueueSnackbar('Please fill in all fields', { variant: 'warning' });
      return;
    }
    setLoading(true);
    try {
      await login(username, password);
      enqueueSnackbar('Login successful!', { variant: 'success' });
      navigate('/dashboard');
    } catch (error: any) {
      enqueueSnackbar(error.response?.data?.message || error.message || 'Login failed', { variant: 'error' });
    } finally {
      setLoading(false);
    }
  };

  return (
    <Box
      sx={{
        minHeight: '100vh',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        position: 'relative',
        overflow: 'hidden',
        background: 'linear-gradient(135deg, #0a0e1a 0%, #0f172a 30%, #1e1b4b 70%, #0a0e1a 100%)',
      }}
    >
      {/* Animated background elements */}
      <Box sx={{
        position: 'absolute', width: 400, height: 400, borderRadius: '50%',
        background: 'radial-gradient(circle, rgba(99,102,241,0.15) 0%, transparent 70%)',
        top: -100, right: -100, animation: `${pulse} 4s ease-in-out infinite`,
      }} />
      <Box sx={{
        position: 'absolute', width: 300, height: 300, borderRadius: '50%',
        background: 'radial-gradient(circle, rgba(6,182,212,0.1) 0%, transparent 70%)',
        bottom: -50, left: -50, animation: `${pulse} 5s ease-in-out infinite 1s`,
      }} />
      <Box sx={{
        position: 'absolute', width: 200, height: 200, borderRadius: 4,
        border: `1px solid ${alpha('#6366f1', 0.1)}`,
        top: '20%', left: '15%', animation: `${float} 6s ease-in-out infinite`,
        transform: 'rotate(45deg)',
      }} />
      <Box sx={{
        position: 'absolute', width: 120, height: 120, borderRadius: 3,
        border: `1px solid ${alpha('#06b6d4', 0.08)}`,
        bottom: '25%', right: '20%', animation: `${float} 8s ease-in-out infinite 2s`,
      }} />

      <Card
        sx={{
          width: '100%',
          maxWidth: 420,
          mx: 2,
          position: 'relative',
          zIndex: 1,
          backgroundColor: alpha('#111827', 0.7),
          backdropFilter: 'blur(40px)',
          border: `1px solid ${alpha('#94a3b8', 0.1)}`,
          borderRadius: 4,
          boxShadow: `0 24px 64px ${alpha('#000', 0.4)}, 0 0 80px ${alpha('#6366f1', 0.05)}`,
          '&:hover': { transform: 'none' },
        }}
      >
        <CardContent sx={{ p: 5 }}>
          {/* Logo */}
          <Box sx={{ textAlign: 'center', mb: 4 }}>
            <Box
              sx={{
                width: 64, height: 64, borderRadius: 3, mx: 'auto', mb: 2,
                background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
                display: 'flex', alignItems: 'center', justifyContent: 'center',
                fontSize: '1.5rem', fontWeight: 800, color: '#fff',
                boxShadow: `0 8px 32px ${alpha('#6366f1', 0.3)}`,
              }}
            >
              FW
            </Box>
            <Typography variant="h5" sx={{ fontWeight: 800, color: '#f1f5f9', mb: 0.5 }}>
              Firmware Manager
            </Typography>
            <Typography variant="body2" sx={{ color: '#64748b' }}>
              Sign in to manage your firmware
            </Typography>
          </Box>

          <form onSubmit={handleSubmit}>
            <TextField
              id="login-username"
              fullWidth label="Username" value={username}
              onChange={(e) => setUsername(e.target.value)}
              sx={{ mb: 2.5 }} autoFocus autoComplete="username"
            />
            <TextField
              id="login-password"
              fullWidth label="Password" type={showPassword ? 'text' : 'password'}
              value={password} onChange={(e) => setPassword(e.target.value)}
              sx={{ mb: 3 }} autoComplete="current-password"
              InputProps={{
                endAdornment: (
                  <InputAdornment position="end">
                    <IconButton onClick={() => setShowPassword(!showPassword)} edge="end" size="small">
                      {showPassword ? <VisibilityOff /> : <Visibility />}
                    </IconButton>
                  </InputAdornment>
                ),
              }}
            />
            <Button
              id="login-submit"
              type="submit" fullWidth variant="contained" disabled={loading}
              sx={{
                py: 1.5, fontSize: '0.95rem', fontWeight: 700,
                background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
                boxShadow: `0 4px 24px ${alpha('#6366f1', 0.3)}`,
                '&:hover': {
                  background: 'linear-gradient(135deg, #4f46e5 0%, #7c3aed 100%)',
                  boxShadow: `0 8px 32px ${alpha('#6366f1', 0.4)}`,
                },
              }}
              startIcon={loading ? <CircularProgress size={20} color="inherit" /> : <LoginIcon />}
            >
              {loading ? 'Signing in...' : 'Sign In'}
            </Button>
          </form>

          <Typography variant="caption" sx={{ display: 'block', textAlign: 'center', mt: 3, color: '#475569' }}>
            Default: admin / admin123
          </Typography>
        </CardContent>
      </Card>
    </Box>
  );
};

export default LoginPage;
