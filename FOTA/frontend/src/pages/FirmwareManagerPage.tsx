import React, { useState } from 'react';
import { Box, Typography, Fab, Tooltip } from '@mui/material';
import { CloudUpload as UploadIcon } from '@mui/icons-material';
import FirmwareTable from '../components/FirmwareTable';
import FirmwareUpload from '../components/FirmwareUpload';
import { useAuth } from '../contexts/AuthContext';

const FirmwareManagerPage: React.FC = () => {
  const [uploadOpen, setUploadOpen] = useState(false);
  const [refreshTrigger, setRefreshTrigger] = useState(0);
  const { isAdmin } = useAuth();

  return (
    <Box>
      <Box sx={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', mb: 3 }}>
        <Typography variant="h4" sx={{ fontWeight: 800, color: '#f1f5f9' }}>
          Firmware Manager
        </Typography>
      </Box>

      <FirmwareTable refreshTrigger={refreshTrigger} />

      {isAdmin && (
        <>
          <Tooltip title="Upload Firmware">
            <Fab
              color="primary"
              onClick={() => setUploadOpen(true)}
              sx={{
                position: 'fixed', bottom: 32, right: 32,
                background: 'linear-gradient(135deg, #6366f1 0%, #8b5cf6 100%)',
                boxShadow: '0 8px 32px rgba(99,102,241,0.3)',
                '&:hover': {
                  background: 'linear-gradient(135deg, #4f46e5 0%, #7c3aed 100%)',
                  boxShadow: '0 12px 40px rgba(99,102,241,0.4)',
                  transform: 'scale(1.05)',
                },
                transition: 'all 0.2s ease',
              }}
            >
              <UploadIcon />
            </Fab>
          </Tooltip>

          <FirmwareUpload
            open={uploadOpen}
            onClose={() => setUploadOpen(false)}
            onSuccess={() => setRefreshTrigger((t) => t + 1)}
          />
        </>
      )}
    </Box>
  );
};

export default FirmwareManagerPage;
