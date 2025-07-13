// Simple utilities for Smart Package Box Dashboard
// Author: M.Nawval Alfazri

// Helper functions untuk dashboard
function formatTimestamp(timestamp) {
  try {
    const date = new Date(timestamp);
    return date.toLocaleString('id-ID', {
      timeZone: 'Asia/Jakarta',
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    });
  } catch (error) {
    return 'Invalid Date';
  }
}

// Show toast notification
function showSimpleToast(message, type = 'success') {
  const toast = document.createElement('div');
  toast.style.cssText = `
    position: fixed;
    top: 20px;
    left: 50%;
    transform: translateX(-50%);
    background: ${type === 'success' ? '#10b981' : '#ef4444'};
    color: white;
    padding: 12px 24px;
    border-radius: 8px;
    z-index: 9999;
    font-weight: 600;
    box-shadow: 0 4px 12px rgba(0,0,0,0.15);
  `;
  toast.textContent = message;
  
  document.body.appendChild(toast);
  
  setTimeout(() => {
    toast.remove();
  }, 3000);
}

// Check if mobile device
function isMobileDevice() {
  return window.innerWidth <= 768;
}

// Copy text to clipboard
async function copyToClipboard(text) {
  try {
    await navigator.clipboard.writeText(text);
    showSimpleToast('Copied to clipboard!', 'success');
  } catch (error) {
    showSimpleToast('Failed to copy', 'error');
  }
}

// Debounce function
function debounce(func, wait) {
  let timeout;
  return function executedFunction(...args) {
    const later = () => {
      clearTimeout(timeout);
      func(...args);
    };
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
  };
}

// Simple storage wrapper
const simpleStorage = {
  set(key, value) {
    try {
      localStorage.setItem(key, JSON.stringify(value));
      return true;
    } catch (error) {
      console.error('Storage error:', error);
      return false;
    }
  },
  
  get(key, defaultValue = null) {
    try {
      const item = localStorage.getItem(key);
      return item ? JSON.parse(item) : defaultValue;
    } catch (error) {
      console.error('Storage error:', error);
      return defaultValue;
    }
  },
  
  remove(key) {
    try {
      localStorage.removeItem(key);
      return true;
    } catch (error) {
      console.error('Storage error:', error);
      return false;
    }
  }
};

// Network status checker
function checkNetworkStatus() {
  return navigator.onLine;
}

// Add event listeners when DOM is ready
document.addEventListener('DOMContentLoaded', function() {
  console.log('🚀 Smart Package Box utilities loaded');
  
  // Add mobile class if needed
  if (isMobileDevice()) {
    document.body.classList.add('mobile-device');
  }
  
  // Network status listeners
  window.addEventListener('online', () => {
    showSimpleToast('Internet connected', 'success');
  });
  
  window.addEventListener('offline', () => {
    showSimpleToast('Internet disconnected', 'error');
  });
});

// Export functions for global use
window.smartBoxUtils = {
  formatTimestamp,
  showSimpleToast,
  isMobileDevice,
  copyToClipboard,
  debounce,
  simpleStorage,
  checkNetworkStatus
};