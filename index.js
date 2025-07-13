// Optional additional JavaScript for Smart Package Box
// This file can be used for extra features or extensions

console.log('📦 Smart Package Box - Additional features loaded');

// Optional: Advanced chart configurations
const chartDefaults = {
  responsive: true,
  maintainAspectRatio: true,
  plugins: {
    legend: {
      position: 'top'
    }
  }
};

// Optional: Network monitoring
function startNetworkMonitoring() {
  setInterval(() => {
    if (!navigator.onLine) {
      console.warn('⚠️ Network disconnected');
    }
  }, 5000);
}

// Optional: Performance monitoring
function logPerformance() {
  if ('performance' in window) {
    const timing = performance.timing;
    const loadTime = timing.loadEventEnd - timing.navigationStart;
    console.log(`📊 Page load time: ${loadTime}ms`);
  }
}

// Optional: Error logging
function setupErrorLogging() {
  window.addEventListener('error', (event) => {
    console.error('🚨 JavaScript Error:', {
      message: event.message,
      filename: event.filename,
      line: event.lineno,
      column: event.colno
    });
  });
}

// Initialize optional features
document.addEventListener('DOMContentLoaded', () => {
  // Uncomment features you want to enable:
  
  // startNetworkMonitoring();
  // logPerformance();
  // setupErrorLogging();
  
  console.log('✅ Additional features initialized');
});