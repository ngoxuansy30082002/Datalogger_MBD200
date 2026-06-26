/**
 * LAYOUT LOADER - with file:// protocol support
 * Uses jQuery.get() to load sidebar component
 * For local file:// protocol, sidebar is embedded inline as fallback
 */

(function () {
    'use strict';

    function loadSidebar() {
        $.get('components/sidebar.html')
            .done(function(html) {
                $('#sidebarMount').html(html);
                initSidebarState();
                initSidebarActive();
            })
            .fail(function() {
                $('#sidebarMount').html(getSidebarHTML());
                initSidebarState();
                initSidebarActive();
            });
    }

    function initSidebarState() {
        const sidebarCollapsed = localStorage.getItem('sidebarCollapsed') === 'true';
        if (sidebarCollapsed && window.innerWidth > 1024) {
            $('.sidebar').addClass('collapsed');
        }
        if (typeof window.updateSidebarTooltips === 'function') {
            window.updateSidebarTooltips();
        }
    }

    function initSidebarActive() {
        const currentPage = window.location.pathname.split('/').pop() || 'index.html';
        $('.nav-link-item[data-page]').each(function () {
            if ($(this).data('page') === currentPage) {
                $(this).addClass('active');
                const $parentCollapse = $(this).closest('.collapse');
                if ($parentCollapse.length) {
                    $parentCollapse.addClass('show');
                    $parentCollapse.prev('.nav-link-item').attr('aria-expanded', 'true');
                }
            }
        });
    }

    function getSidebarHTML() {
        return `
<aside class="sidebar" id="sidebar">
    <a href="index.html" class="sidebar-brand">
        <div class="brand-icon"><i class="bi bi-cpu-fill"></i></div>
        <div class="brand-text"><h6>DataLogger</h6><small>Monitoring System</small></div>
    </a>
    <nav class="sidebar-nav" id="sidebarNav">

        <div class="nav-label">MAIN</div>
        <div class="nav-item-wrapper">
            <a href="index.html" class="nav-link-item" data-page="index.html">
                <span class="nav-icon"><i class="bi bi-house-door-fill"></i></span>
                <span class="nav-text">Home</span>
            </a>
        </div>
        <div class="nav-item-wrapper">
            <a href="dashboard.html" class="nav-link-item" data-page="dashboard.html">
                <span class="nav-icon"><i class="bi bi-grid-1x2-fill"></i></span>
                <span class="nav-text">Dashboard</span>
            </a>
        </div>

        <div class="nav-label">SENSOR CONFIG</div>
        <div class="nav-item-wrapper">
            <button class="nav-link-item" data-bs-toggle="collapse" data-bs-target="#menuSensorConfig" aria-expanded="false">
                <span class="nav-icon"><i class="bi bi-sliders"></i></span>
                <span class="nav-text">Sensor Configuration</span>
                <i class="bi bi-chevron-right nav-arrow"></i>
            </button>
        </div>
        <div class="collapse" id="menuSensorConfig">
            <ul class="sub-menu list-unstyled">
                <li class="nav-item-wrapper"><a href="sensor-general.html" class="nav-link-item" data-page="sensor-general.html"><span class="nav-text">General Sensor Settings</span></a></li>
                <li class="nav-item-wrapper"><a href="hmi-display.html" class="nav-link-item" data-page="hmi-display.html"><span class="nav-text">HMI Display Configuration</span></a></li>
                <li class="nav-item-wrapper"><a href="rule-engine.html" class="nav-link-item" data-page="rule-engine.html"><span class="nav-text">Rule Engine</span></a></li>
            </ul>
        </div>

        <div class="nav-item-wrapper">
            <button class="nav-link-item" data-bs-toggle="collapse" data-bs-target="#menuSensorTypes" aria-expanded="false">
                <span class="nav-icon"><i class="bi bi-diagram-3-fill"></i></span>
                <span class="nav-text">Sensor Types</span>
                <i class="bi bi-chevron-right nav-arrow"></i>
            </button>
        </div>
        <div class="collapse" id="menuSensorTypes">
            <ul class="sub-menu list-unstyled">
                <li class="nav-item-wrapper"><a href="analog.html" class="nav-link-item" data-page="analog.html"><span class="nav-text">Analog Sensor</span></a></li>
                <li class="nav-item-wrapper"><a href="modbus.html" class="nav-link-item" data-page="modbus.html"><span class="nav-text">Modbus Sensor</span></a></li>
                <li class="nav-item-wrapper"><a href="input-capture.html" class="nav-link-item" data-page="input-capture.html"><span class="nav-text">Input Capture</span></a></li>
                <li class="nav-item-wrapper"><a href="output.html" class="nav-link-item" data-page="output.html"><span class="nav-text">Output Control</span></a></li>
            </ul>
        </div>

        <div class="nav-label">SYSTEM</div>
        <div class="nav-item-wrapper">
            <button class="nav-link-item" data-bs-toggle="collapse" data-bs-target="#menuSystemConfig" aria-expanded="false">
                <span class="nav-icon"><i class="bi bi-gear-fill"></i></span>
                <span class="nav-text">System Configuration</span>
                <i class="bi bi-chevron-right nav-arrow"></i>
            </button>
        </div>
        <div class="collapse" id="menuSystemConfig">
            <ul class="sub-menu list-unstyled">
                <li class="nav-item-wrapper"><a href="user.html" class="nav-link-item" data-page="user.html"><span class="nav-text">User Management</span></a></li>
                <li class="nav-item-wrapper"><a href="time.html" class="nav-link-item" data-page="time.html"><span class="nav-text">Time Settings</span></a></li>
                <li class="nav-item-wrapper"><a href="serial-com.html" class="nav-link-item" data-page="serial-com.html"><span class="nav-text">RS485 Modbus Config</span></a></li>
                <li class="nav-item-wrapper"><a href="network.html" class="nav-link-item" data-page="network.html"><span class="nav-text">Network Configuration</span></a></li>
                <li class="nav-item-wrapper"><a href="sim.html" class="nav-link-item" data-page="sim.html"><span class="nav-text">SIM Config (4G/LTE)</span></a></li>
                <li class="nav-item-wrapper"><a href="ftp.html" class="nav-link-item" data-page="ftp.html"><span class="nav-text">FTP Configuration</span></a></li>
                <li class="nav-item-wrapper"><a href="mqtt.html" class="nav-link-item" data-page="mqtt.html"><span class="nav-text">MQTT Configuration</span></a></li>
                <li class="nav-item-wrapper"><a href="storage.html" class="nav-link-item" data-page="storage.html"><span class="nav-text">Storage (SD Card)</span></a></li>
            </ul>
        </div>
    </nav>

    <div class="sidebar-footer">
        <div class="nav-item-wrapper">
            <a href="#" class="nav-link-item">
                <span class="nav-icon"><i class="bi bi-question-circle"></i></span>
                <span class="nav-text">Help &amp; Support</span>
            </a>
        </div>
    </div>
</aside>
<div class="sidebar-overlay" id="sidebarOverlay"></div>`;
    }

    if (typeof $ !== 'undefined') {
        loadSidebar();
    } else {
        document.addEventListener('DOMContentLoaded', function () {
            loadSidebar();
        });
    }

})();
