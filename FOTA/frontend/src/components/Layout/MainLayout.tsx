import React, { useState } from 'react';
import { Outlet, useLocation, useNavigate } from 'react-router-dom';
import {
  Box,
  Drawer,
  AppBar,
  Toolbar,
  Typography,
  IconButton,
  List,
  ListItem,
  ListItemButton,
  ListItemIcon,
  ListItemText,
  Avatar,
  Menu,
  MenuItem,
  Chip,
  Divider,
  useMediaQuery,
  useTheme,
  alpha,
} from '@mui/material';
import {
  Menu as MenuIcon,
  Dashboard as DashboardIcon,
  Memory as FirmwareIcon,
  Router as MqttIcon,
  Devices as DevicesIcon,
  Logout as LogoutIcon,
  Person as PersonIcon,
  FiberManualRecord as StatusDot,
} from '@mui/icons-material';
import { useAuth } from '../../contexts/AuthContext';
import { useSocket } from '../../contexts/SocketContext';

const DRAWER_WIDTH = 280;

const navItems = [
  { path: '/dashboard', label: 'Dashboard', icon: <DashboardIcon /> },
  { path: '/firmware', label: 'Firmware Manager', icon: <FirmwareIcon /> },
  { path: '/mqtt', label: 'MQTT Monitor', icon: <MqttIcon /> },
  { path: '/devices', label: 'Devices', icon: <DevicesIcon /> },
];

const MainLayout: React.FC = () => {
  const theme = useTheme();
  const isMobile = useMediaQuery(theme.breakpoints.down('md'));
  const [mobileOpen, setMobileOpen] = useState(false);
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  const location = useLocation();
  const navigate = useNavigate();
  const { user, logout, isAdmin } = useAuth();
  const { isConnected } = useSocket();

  const handleDrawerToggle = () => setMobileOpen(!mobileOpen);
  const handleMenuOpen = (event: React.MouseEvent<HTMLElement>) => setAnchorEl(event.currentTarget);
  const handleMenuClose = () => setAnchorEl(null);

  const handleLogout = () => {
    handleMenuClose();
    logout();
    navigate('/login');
  };

  const drawer = (
    <Box sx={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      {/* Logo */}
      <Box
        sx={{
          p: 3,
          display: 'flex',
          alignItems: 'center',
          gap: 2,
        }}
      >
        <Box
          sx={{
            width: 40,
            height: 40,
            borderRadius: 2,
            background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            fontSize: '1.2rem',
            fontWeight: 800,
            color: '#fff',
          }}
        >
          FW
        </Box>
        <Box>
          <Typography variant="subtitle1" sx={{ fontWeight: 700, lineHeight: 1.2, color: '#f1f5f9' }}>
            Firmware
          </Typography>
          <Typography variant="caption" sx={{ color: '#64748b', fontWeight: 500 }}>
            Management Server
          </Typography>
        </Box>
      </Box>

      <Divider sx={{ borderColor: alpha('#94a3b8', 0.08) }} />

      {/* Navigation */}
      <List sx={{ flex: 1, px: 2, py: 2 }}>
        {navItems.map((item) => {
          const isActive = location.pathname === item.path;
          return (
            <ListItem key={item.path} disablePadding sx={{ mb: 0.5 }}>
              <ListItemButton
                onClick={() => {
                  navigate(item.path);
                  if (isMobile) setMobileOpen(false);
                }}
                sx={{
                  borderRadius: 2,
                  py: 1.2,
                  px: 2,
                  transition: 'all 0.2s ease',
                  backgroundColor: isActive ? alpha('#6366f1', 0.15) : 'transparent',
                  borderLeft: isActive ? '3px solid #6366f1' : '3px solid transparent',
                  '&:hover': {
                    backgroundColor: isActive ? alpha('#6366f1', 0.2) : alpha('#94a3b8', 0.05),
                  },
                }}
              >
                <ListItemIcon
                  sx={{
                    minWidth: 40,
                    color: isActive ? '#818cf8' : '#64748b',
                    transition: 'color 0.2s ease',
                  }}
                >
                  {item.icon}
                </ListItemIcon>
                <ListItemText
                  primary={item.label}
                  primaryTypographyProps={{
                    fontSize: '0.875rem',
                    fontWeight: isActive ? 600 : 500,
                    color: isActive ? '#f1f5f9' : '#94a3b8',
                  }}
                />
              </ListItemButton>
            </ListItem>
          );
        })}
      </List>

      {/* Status footer */}
      <Box sx={{ p: 2, borderTop: `1px solid ${alpha('#94a3b8', 0.08)}` }}>
        <Box sx={{ display: 'flex', alignItems: 'center', gap: 1, mb: 1 }}>
          <StatusDot sx={{ fontSize: 10, color: isConnected ? '#10b981' : '#ef4444' }} />
          <Typography variant="caption" sx={{ color: '#64748b' }}>
            {isConnected ? 'Real-time connected' : 'Disconnected'}
          </Typography>
        </Box>
      </Box>
    </Box>
  );

  return (
    <Box sx={{ display: 'flex', minHeight: '100vh', background: '#0a0e1a' }}>
      {/* Sidebar */}
      <Box component="nav" sx={{ width: { md: DRAWER_WIDTH }, flexShrink: { md: 0 } }}>
        {isMobile ? (
          <Drawer
            variant="temporary"
            open={mobileOpen}
            onClose={handleDrawerToggle}
            ModalProps={{ keepMounted: true }}
            sx={{
              '& .MuiDrawer-paper': { width: DRAWER_WIDTH },
            }}
          >
            {drawer}
          </Drawer>
        ) : (
          <Drawer
            variant="permanent"
            open
            sx={{
              '& .MuiDrawer-paper': { width: DRAWER_WIDTH, boxSizing: 'border-box' },
            }}
          >
            {drawer}
          </Drawer>
        )}
      </Box>

      {/* Main content */}
      <Box sx={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
        {/* App Bar */}
        <AppBar
          position="sticky"
          elevation={0}
          sx={{
            backgroundColor: alpha('#0a0e1a', 0.8),
            backdropFilter: 'blur(20px)',
            borderBottom: `1px solid ${alpha('#94a3b8', 0.08)}`,
          }}
        >
          <Toolbar sx={{ justifyContent: 'space-between' }}>
            <Box sx={{ display: 'flex', alignItems: 'center', gap: 1 }}>
              {isMobile && (
                <IconButton color="inherit" edge="start" onClick={handleDrawerToggle}>
                  <MenuIcon />
                </IconButton>
              )}
              <Typography variant="h6" sx={{ fontWeight: 600, fontSize: '1.1rem' }}>
                {navItems.find((item) => item.path === location.pathname)?.label || 'Dashboard'}
              </Typography>
            </Box>

            <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
              <Chip
                size="small"
                label={isAdmin ? 'Admin' : 'User'}
                sx={{
                  backgroundColor: isAdmin ? alpha('#6366f1', 0.15) : alpha('#06b6d4', 0.15),
                  color: isAdmin ? '#818cf8' : '#22d3ee',
                  fontWeight: 600,
                  fontSize: '0.75rem',
                }}
              />
              <IconButton onClick={handleMenuOpen} size="small">
                <Avatar
                  sx={{
                    width: 36,
                    height: 36,
                    background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
                    fontSize: '0.875rem',
                    fontWeight: 700,
                  }}
                >
                  {user?.username?.charAt(0).toUpperCase()}
                </Avatar>
              </IconButton>
              <Menu
                anchorEl={anchorEl}
                open={Boolean(anchorEl)}
                onClose={handleMenuClose}
                transformOrigin={{ horizontal: 'right', vertical: 'top' }}
                anchorOrigin={{ horizontal: 'right', vertical: 'bottom' }}
              >
                <MenuItem disabled>
                  <PersonIcon sx={{ mr: 1, fontSize: 20 }} />
                  {user?.username}
                </MenuItem>
                <Divider />
                <MenuItem onClick={handleLogout}>
                  <LogoutIcon sx={{ mr: 1, fontSize: 20 }} />
                  Logout
                </MenuItem>
              </Menu>
            </Box>
          </Toolbar>
        </AppBar>

        {/* Page content */}
        <Box
          component="main"
          sx={{
            flex: 1,
            overflow: 'auto',
            p: { xs: 2, sm: 3 },
          }}
        >
          <Outlet />
        </Box>
      </Box>
    </Box>
  );
};

export default MainLayout;
