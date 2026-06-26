import React from 'react';
import { Box, Card, CardContent, Typography, alpha } from '@mui/material';

interface StatsCardProps {
  title: string;
  value: string | number;
  icon: React.ReactNode;
  color: string;
  subtitle?: string;
}

const StatsCard: React.FC<StatsCardProps> = ({ title, value, icon, color, subtitle }) => {
  return (
    <Card
      sx={{
        position: 'relative',
        overflow: 'hidden',
        '&::before': {
          content: '""',
          position: 'absolute',
          top: 0,
          left: 0,
          right: 0,
          height: 3,
          background: `linear-gradient(90deg, ${color}, ${alpha(color, 0.5)})`,
        },
      }}
    >
      <CardContent sx={{ p: 3 }}>
        <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'flex-start' }}>
          <Box>
            <Typography
              variant="body2"
              sx={{
                color: '#64748b',
                fontWeight: 500,
                textTransform: 'uppercase',
                fontSize: '0.7rem',
                letterSpacing: '0.05em',
                mb: 1,
              }}
            >
              {title}
            </Typography>
            <Typography
              variant="h3"
              sx={{
                fontWeight: 800,
                color: '#f1f5f9',
                lineHeight: 1,
                mb: subtitle ? 0.5 : 0,
                fontSize: { xs: '1.8rem', sm: '2.2rem' },
              }}
            >
              {typeof value === 'number' ? value.toLocaleString() : value}
            </Typography>
            {subtitle && (
              <Typography variant="caption" sx={{ color: '#64748b' }}>
                {subtitle}
              </Typography>
            )}
          </Box>
          <Box
            sx={{
              width: 48,
              height: 48,
              borderRadius: 2,
              backgroundColor: alpha(color, 0.1),
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              color: color,
              '& svg': { fontSize: 24 },
            }}
          >
            {icon}
          </Box>
        </Box>
      </CardContent>
    </Card>
  );
};

export default StatsCard;
