/**
 * DATA LOGGER SYSTEM - Main JavaScript
 * Author: Data Logger Frontend
 * Uses: jQuery + Bootstrap 5
 */

/* =============================================
   GLOBAL SYSTEM TIME INTERCEPTOR
   ============================================= */
window.parseAndDisplaySystemTime = function(xmlData) {
    if (!xmlData) return;
    try {
        var sysNode = xmlData.getElementsByTagName('systime')[0];
        if (sysNode && sysNode.firstChild) {
            var sysStr = sysNode.firstChild.nodeValue;
            if (sysStr && sysStr !== '~systime~') {
                var timeStr = sysStr;
                try {
                    var systimeObj = JSON.parse(sysStr);
                    if (systimeObj.systemTime) timeStr = systimeObj.systemTime;
                } catch(e) {
                    // Not JSON, assume raw string
                }
                timeStr = timeStr.replace('T', ' '); // format nicely
                
                var $headerTime = document.getElementById('headerSystemTime');
                if ($headerTime) $headerTime.innerText = timeStr;
            }
        }
    } catch(e) {
        console.error("Error updating system time:", e);
    }
};

// Patch newAJAXCommand to automatically extract system time globally
if (typeof window.newAJAXCommand === 'function') {
    var originalNewAJAXCommand = window.newAJAXCommand;
    window.newAJAXCommand = function(url, container, repeat, data) {
        if (typeof container === 'function') {
            var originalContainer = container;
            container = function(xmlData) {
                window.parseAndDisplaySystemTime(xmlData);
                originalContainer(xmlData);
            };
        }
        originalNewAJAXCommand(url, container, repeat, data);
    };
}

$(document).ready(function () {
    /* =============================================
       SIDEBAR TOGGLE
       ============================================= */
    const isMobile = () => window.innerWidth <= 1024;

    // Load sidebar state from localStorage (main wrapper only, sidebar is in layout.js)
    const sidebarCollapsed = localStorage.getItem('sidebarCollapsed') === 'true';
    if (sidebarCollapsed && !isMobile()) {
        $('.main-wrapper').addClass('sidebar-collapsed');
    }

    // Toggle Sidebar Button
    $(document).on('click', '#toggleSidebar', function () {
        const $sidebar = $('.sidebar');
        const $mainWrapper = $('.main-wrapper');
        const $overlay = $('.sidebar-overlay');

        if (isMobile()) {
            $sidebar.toggleClass('mobile-open');
            $overlay.toggleClass('active');
        } else {
            $sidebar.toggleClass('collapsed');
            $mainWrapper.toggleClass('sidebar-collapsed');
            localStorage.setItem('sidebarCollapsed', $sidebar.hasClass('collapsed'));
        }
    });

    // Close sidebar when overlay clicked (mobile)
    $(document).on('click', '.sidebar-overlay', function () {
        $('.sidebar').removeClass('mobile-open');
        $('.sidebar-overlay').removeClass('active');
    });

    // Handle resize
    $(window).on('resize', function () {
        if (!isMobile()) {
            $('.sidebar').removeClass('mobile-open');
            $('.sidebar-overlay').removeClass('active');
        }
    });

    /* =============================================
       ACTIVE MENU HIGHLIGHT
       ============================================= */
    const currentPage = window.location.pathname.split('/').pop() || 'index.html';

    $('.nav-link-item[data-page]').each(function () {
        const page = $(this).data('page');
        if (page === currentPage) {
            $(this).addClass('active');
            // If inside a submenu, expand the parent
            const $parentCollapse = $(this).closest('.collapse');
            if ($parentCollapse.length) {
                $parentCollapse.addClass('show');
                $parentCollapse.prev('.nav-link-item').attr('aria-expanded', 'true');
            }
        }
    });

    /* =============================================
       SUBMENU COLLAPSE
       ============================================= */
    // Bootstrap already handles collapse via data-bs-toggle
    // We animate the arrow icon
    $(document).on('click', '.nav-link-item[data-bs-toggle="collapse"]', function () {
        // Arrow animation handled by CSS [aria-expanded="true"]
    });

    /* =============================================
       FORM VALIDATION
       ============================================= */
    $(document).on('submit', 'form.needs-validation', function (e) {
        e.preventDefault();
        e.stopPropagation();

        const $form = $(this);
        let isValid = true;

        // Clear previous validation states
        $form.find('.form-control, .form-select').removeClass('is-invalid is-valid');
        $form.find('.invalid-feedback').remove();

        // Validate required fields
        $form.find('[required]').each(function () {
            const $field = $(this);
            const val = $field.val().trim();

            if (!val) {
                $field.addClass('is-invalid');
                isValid = false;
                if (!$field.next('.invalid-feedback').length) {
                    $field.after('<div class="invalid-feedback">Trường này không được để trống.</div>');
                }
            } else {
                $field.addClass('is-valid');

                // Email validation
                if ($field.attr('type') === 'email') {
                    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
                    if (!emailRegex.test(val)) {
                        $field.removeClass('is-valid').addClass('is-invalid');
                        isValid = false;
                        $field.after('<div class="invalid-feedback">Email không hợp lệ.</div>');
                    }
                }

                // IP Address validation
                if ($field.hasClass('ip-field')) {
                    const ipRegex = /^(\d{1,3}\.){3}\d{1,3}$/;
                    if (!ipRegex.test(val)) {
                        $field.removeClass('is-valid').addClass('is-invalid');
                        isValid = false;
                        $field.after('<div class="invalid-feedback">Địa chỉ IP không hợp lệ.</div>');
                    }
                }

                // Port validation
                if ($field.hasClass('port-field')) {
                    const port = parseInt(val);
                    if (isNaN(port) || port < 1 || port > 65535) {
                        $field.removeClass('is-valid').addClass('is-invalid');
                        isValid = false;
                        $field.after('<div class="invalid-feedback">Port phải từ 1 đến 65535.</div>');
                    }
                }
            }
        });

        if (isValid) {
            const formData = {};
            $form.serializeArray().forEach(item => {
                formData[item.name] = item.value;
            });

            const formId = $form.attr('id') || 'form';
            console.log(`[${formId}] Form submitted:`, formData);

            // showToast('Lưu cấu hình thành công!', 'success');

            // Button loading animation
            const $submitBtn = $form.find('[type="submit"]');
            const originalHtml = $submitBtn.html();
            $submitBtn.html('<span class="spinner-border spinner-border-sm me-2"></span>Đang lưu...').prop('disabled', true);
            setTimeout(() => {
                $submitBtn.html(originalHtml).prop('disabled', false);
                $form.find('.form-control, .form-select').removeClass('is-valid is-invalid');
            }, 1500);
        } else {
            showToast('Vui lòng kiểm tra lại các trường nhập liệu.', 'error');
            $form.find('.is-invalid').first().focus();
        }
    });

    // Live validation - clear error on input
    $(document).on('input change', '.form-control, .form-select', function () {
        if ($(this).hasClass('is-invalid') && $(this).val().trim()) {
            $(this).removeClass('is-invalid').addClass('is-valid');
            $(this).next('.invalid-feedback').remove();
        }
    });

    /* =============================================
       RESET FORM
       ============================================= */
    $(document).on('click', '.btn-reset', function () {
        const $form = $(this).closest('form');
        $form[0].reset();
        $form.find('.form-control, .form-select').removeClass('is-valid is-invalid');
        $form.find('.invalid-feedback').remove();
        showToast('Đã đặt lại về mặc định.', 'info');
    });

    /* =============================================
       TOAST NOTIFICATIONS
       ============================================= */
    window.showToast = function (message, type = 'success', duration = 3500) {
        const icons = {
            success: '<i class="bi bi-check-circle-fill text-success"></i>',
            error: '<i class="bi bi-x-circle-fill text-danger"></i>',
            warning: '<i class="bi bi-exclamation-triangle-fill text-warning"></i>',
            info: '<i class="bi bi-info-circle-fill text-primary"></i>'
        };

        if (!$('.toast-container').length) {
            $('body').append('<div class="toast-container"></div>');
        }

        const toastId = 'toast_' + Date.now();
        const $toast = $(`
            <div class="toast-custom toast-${type}" id="${toastId}">
                <div style="font-size:18px">${icons[type]}</div>
                <div style="flex:1">
                    <div style="font-size:13.5px;font-weight:600;color:#1f2937;margin-bottom:2px">
                        ${type === 'success' ? 'Thành công' : type === 'error' ? 'Lỗi' : type === 'warning' ? 'Cảnh báo' : 'Thông báo'}
                    </div>
                    <div style="font-size:13px;color:#6b7280">${message}</div>
                </div>
                <button style="border:none;background:none;color:#9ca3af;cursor:pointer;padding:0;font-size:14px" onclick="$('#${toastId}').fadeOut(300, function(){$(this).remove();})">
                    <i class="bi bi-x"></i>
                </button>
            </div>
        `);

        $('.toast-container').append($toast);

        setTimeout(() => {
            $toast.fadeOut(300, function () { $(this).remove(); });
        }, duration);
    };

    /* =============================================
       TABLE - CONFIRM DELETE
       ============================================= */
    $(document).on('click', '.btn-delete-row', function () {
        const $row = $(this).closest('tr');
        const name = $row.find('td:first').text().trim();

        if (confirm(`Bạn có chắc muốn xóa "${name}"?`)) {
            $row.fadeOut(300, function () {
                $(this).remove();
                showToast(`Đã xóa "${name}" thành công.`, 'success');
            });
            console.log('[Table] Deleted row:', name);
        }
    });

    /* =============================================
       PASSWORD TOGGLE
       ============================================= */
    $(document).on('click', '.toggle-password', function () {
        const $input = $(this).siblings('input');
        const $icon = $(this).find('i');

        if ($input.attr('type') === 'password') {
            $input.attr('type', 'text');
            $icon.removeClass('bi-eye').addClass('bi-eye-slash');
        } else {
            $input.attr('type', 'password');
            $icon.removeClass('bi-eye-slash').addClass('bi-eye');
        }
    });

    /* =============================================
       CURRENT DATETIME DISPLAY (FROM BACKEND)
       ============================================= */
    // Tạo UI trong header nếu chưa có
    if (!$('#headerSystemTime').length && $('.header-actions').length) {
        $('.header-actions').prepend(`
            <div class="system-time-display d-none d-sm-flex align-items-center" style="background:#f8fafc; border:1px solid #e2e8f0; border-radius:6px; padding:4px 12px; margin-right:8px;">
                <i class="bi bi-clock-history text-primary me-2"></i>
                <span id="headerSystemTime" style="font-family:'Consolas',monospace; font-size:13px; font-weight:700; color:#0f172a; letter-spacing:0.5px;">--:--:--</span>
            </div>
        `);
    }

    // If the page doesn't have an existing updatedata function to poll, we should poll xml/systime.xml globally
    // if (typeof window.updatedata !== 'function') {
        // No updatedata means no existing AJAX polling loop for this page.
        // We will create a standalone polling loop just for system time.
        function globalTimePoll() {
            if (typeof window.newAJAXCommand === 'function') {
                window.updatedata = function(xmlData) {}; // Dummy updatedata so patched newAJAXCommand intercepts it
                newAJAXCommand('xml/systime.xml', window.updatedata, true);
            } else {
                // Fallback using jQuery if mchp.js is not loaded
                $.ajax({
                    url: 'xml/systime.xml',
                    type: 'GET',
                    dataType: 'xml',
                    cache: false,
                    success: function(xmlData) {
                        window.parseAndDisplaySystemTime(xmlData);
                    },
                    complete: function() {
                        setTimeout(globalTimePoll, 500);
                    }
                });
            }
        }
        setTimeout(globalTimePoll, 500);
    // }

    /* =============================================
       TOOLTIPS INIT
       ============================================= */
    const tooltipEls = document.querySelectorAll('[data-bs-toggle="tooltip"]');
    tooltipEls.forEach(el => new bootstrap.Tooltip(el, { trigger: 'hover' }));

    /* =============================================
       SIDEBAR TOOLTIP ON COLLAPSED
       ============================================= */
    window.updateSidebarTooltips = function() {
        const isCollapsed = $('.sidebar').hasClass('collapsed');
        $('.nav-link-item[data-page], .nav-link-item[data-bs-toggle="collapse"]').each(function () {
            if (isCollapsed && !isMobile()) {
                const label = $(this).find('.nav-text').text().trim();
                $(this).attr('data-bs-toggle-old', 'tooltip'); // we handle manually
                $(this).attr('title', label);
            } else {
                $(this).removeAttr('title');
            }
        });
    }

    $(document).on('click', '#toggleSidebar', window.updateSidebarTooltips);
});
