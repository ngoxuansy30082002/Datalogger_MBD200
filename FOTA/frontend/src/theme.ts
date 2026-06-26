import { createTheme, alpha } from '@mui/material/styles';

export const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#6366f1',
      light: '#818cf8',
      dark: '#4f46e5',
    },
    secondary: {
      main: '#06b6d4',
      light: '#22d3ee',
      dark: '#0891b2',
    },
    success: {
      main: '#10b981',
      light: '#34d399',
      dark: '#059669',
    },
    warning: {
      main: '#f59e0b',
      light: '#fbbf24',
      dark: '#d97706',
    },
    error: {
      main: '#ef4444',
      light: '#f87171',
      dark: '#dc2626',
    },
    background: {
      default: '#0a0e1a',
      paper: '#111827',
    },
    text: {
      primary: '#f1f5f9',
      secondary: '#94a3b8',
    },
    divider: alpha('#94a3b8', 0.12),
  },
  typography: {
    fontFamily: '"Inter", "Roboto", "Helvetica", "Arial", sans-serif',
    h1: { fontWeight: 800, letterSpacing: '-0.025em' },
    h2: { fontWeight: 700, letterSpacing: '-0.025em' },
    h3: { fontWeight: 700, letterSpacing: '-0.01em' },
    h4: { fontWeight: 600 },
    h5: { fontWeight: 600 },
    h6: { fontWeight: 600 },
    subtitle1: { fontWeight: 500 },
    subtitle2: { fontWeight: 500, color: '#94a3b8' },
    body1: { fontSize: '0.938rem' },
    body2: { fontSize: '0.875rem', color: '#94a3b8' },
    button: { fontWeight: 600, textTransform: 'none' },
  },
  shape: {
    borderRadius: 12,
  },
  components: {
    MuiCssBaseline: {
      styleOverrides: {
        body: {
          scrollbarColor: '#1e293b #0a0e1a',
          '&::-webkit-scrollbar': { width: 8 },
          '&::-webkit-scrollbar-track': { background: '#0a0e1a' },
          '&::-webkit-scrollbar-thumb': {
            background: '#1e293b',
            borderRadius: 4,
          },
        },
      },
    },
    MuiPaper: {
      styleOverrides: {
        root: {
          backgroundImage: 'none',
          backgroundColor: alpha('#111827', 0.8),
          backdropFilter: 'blur(20px)',
          border: `1px solid ${alpha('#94a3b8', 0.08)}`,
        },
      },
    },
    MuiCard: {
      styleOverrides: {
        root: {
          backgroundImage: 'none',
          backgroundColor: alpha('#111827', 0.6),
          backdropFilter: 'blur(20px)',
          border: `1px solid ${alpha('#94a3b8', 0.08)}`,
          transition: 'transform 0.2s ease, box-shadow 0.2s ease',
          '&:hover': {
            transform: 'translateY(-2px)',
            boxShadow: `0 8px 32px ${alpha('#6366f1', 0.15)}`,
          },
        },
      },
    },
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 10,
          padding: '8px 20px',
          fontSize: '0.875rem',
          fontWeight: 600,
          boxShadow: 'none',
          '&:hover': {
            boxShadow: 'none',
          },
        },
        contained: {
          background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
          '&:hover': {
            background: 'linear-gradient(135deg, #4f46e5 0%, #7c3aed 100%)',
          },
        },
        outlined: {
          borderColor: alpha('#6366f1', 0.5),
          '&:hover': {
            borderColor: '#6366f1',
            backgroundColor: alpha('#6366f1', 0.08),
          },
        },
      },
    },
    MuiTextField: {
      styleOverrides: {
        root: {
          '& .MuiOutlinedInput-root': {
            borderRadius: 10,
            backgroundColor: alpha('#1e293b', 0.5),
            '& fieldset': {
              borderColor: alpha('#94a3b8', 0.15),
            },
            '&:hover fieldset': {
              borderColor: alpha('#6366f1', 0.5),
            },
            '&.Mui-focused fieldset': {
              borderColor: '#6366f1',
            },
          },
        },
      },
    },
    MuiChip: {
      styleOverrides: {
        root: {
          borderRadius: 8,
          fontWeight: 500,
        },
      },
    },
    MuiTableCell: {
      styleOverrides: {
        root: {
          borderBottom: `1px solid ${alpha('#94a3b8', 0.08)}`,
        },
        head: {
          fontWeight: 600,
          color: '#94a3b8',
          textTransform: 'uppercase',
          fontSize: '0.75rem',
          letterSpacing: '0.05em',
        },
      },
    },
    MuiDrawer: {
      styleOverrides: {
        paper: {
          backgroundColor: '#0f1629',
          borderRight: `1px solid ${alpha('#94a3b8', 0.08)}`,
        },
      },
    },
    MuiDialog: {
      styleOverrides: {
        paper: {
          backgroundColor: '#111827',
          backgroundImage: 'none',
          border: `1px solid ${alpha('#94a3b8', 0.1)}`,
        },
      },
    },
    MuiTooltip: {
      styleOverrides: {
        tooltip: {
          backgroundColor: '#1e293b',
          border: `1px solid ${alpha('#94a3b8', 0.1)}`,
          fontSize: '0.8rem',
        },
      },
    },
    MuiLinearProgress: {
      styleOverrides: {
        root: {
          borderRadius: 4,
          backgroundColor: alpha('#6366f1', 0.15),
        },
        bar: {
          borderRadius: 4,
          background: 'linear-gradient(90deg, #6366f1, #8b5cf6)',
        },
      },
    },
  },
});
