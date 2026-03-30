import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, CheckButtons, Button
from scipy.signal import butter, filtfilt

# --- Defaults ---
DEFAULTS = {
    "amplitude": 0.87,
    "frequency": 0.267,
    "phase": 0.0,
    "noise_mean": 1.10,
    "noise_covariance": 0.101,
    "cutoff_freq": 5.0,
}

t = np.linspace(0, 10, 1000)
fs = 1 / (t[1] - t[0])  # sampling frequency

# Cached noise array — regenerated only when noise params change
noise = np.random.normal(DEFAULTS["noise_mean"],
                         np.sqrt(DEFAULTS["noise_covariance"]),
                         len(t))


def harmonic_with_noise(amplitude, frequency, phase, noise_mean,
                        noise_covariance, show_noise):
    """Generate harmonic y(t) = A*sin(ωt + φ) optionally with noise."""
    clean = amplitude * np.sin(2 * np.pi * frequency * t + phase)
    if show_noise:
        return clean + noise
    return clean


def apply_filter(signal, cutoff_freq):
    """Apply Butterworth low-pass filter."""
    nyq = fs / 2
    norm_cutoff = min(cutoff_freq / nyq, 0.99)
    if norm_cutoff <= 0:
        return signal
    b, a = butter(4, norm_cutoff, btype='low')
    return filtfilt(b, a, signal)


# --- Figure layout ---
fig, ax = plt.subplots(figsize=(10, 6))
fig.subplots_adjust(bottom=0.48)
ax.set_xlim(0, 10)
ax.set_xlabel("t")
ax.set_ylabel("y(t)")
ax.set_title("Гармоніка з шумом та фільтрацією")
ax.grid(True, alpha=0.3)

# Initial signals
noisy_signal = harmonic_with_noise(DEFAULTS["amplitude"], DEFAULTS["frequency"],
                                   DEFAULTS["phase"], DEFAULTS["noise_mean"],
                                   DEFAULTS["noise_covariance"], True)
clean_signal = harmonic_with_noise(DEFAULTS["amplitude"], DEFAULTS["frequency"],
                                   DEFAULTS["phase"], DEFAULTS["noise_mean"],
                                   DEFAULTS["noise_covariance"], False)
filtered_signal = apply_filter(noisy_signal, DEFAULTS["cutoff_freq"])

line_noisy, = ax.plot(t, noisy_signal, color="orange", alpha=0.5,
                       label="Зашумлена")
line_clean, = ax.plot(t, clean_signal, color="blue", linewidth=2,
                       label="Чиста гармоніка")
line_filtered, = ax.plot(t, filtered_signal, color="red", linewidth=1.5,
                          linestyle="--", label="Відфільтрована")
ax.legend(loc="upper right")

# --- Sliders ---
slider_specs = [
    ("Amplitude",        0.01, 3.0,  DEFAULTS["amplitude"]),
    ("Frequency",        0.01, 2.0,  DEFAULTS["frequency"]),
    ("Phase",            0.0,  2*np.pi, DEFAULTS["phase"]),
    ("Noise Mean",       0.0,  3.0,  DEFAULTS["noise_mean"]),
    ("Noise Covariance", 0.001, 1.0, DEFAULTS["noise_covariance"]),
    ("Cutoff Frequency", 0.5,  50.0, DEFAULTS["cutoff_freq"]),
]

sliders = {}
for i, (name, vmin, vmax, vinit) in enumerate(slider_specs):
    ax_slider = fig.add_axes([0.20, 0.34 - i * 0.05, 0.65, 0.03])
    sliders[name] = Slider(ax_slider, name, vmin, vmax, valinit=vinit)

# --- Checkboxes ---
ax_check = fig.add_axes([0.02, 0.01, 0.15, 0.10])
check = CheckButtons(ax_check, ["Show Noise", "Show Filtered"],
                     [True, True])

# --- Reset button ---
ax_reset = fig.add_axes([0.80, 0.01, 0.10, 0.04])
btn_reset = Button(ax_reset, "Reset")

# --- State ---
show_noise = True
show_filtered = True


def update(_=None):
    global noise, show_noise, show_filtered

    amp = sliders["Amplitude"].val
    freq = sliders["Frequency"].val
    phase = sliders["Phase"].val
    n_mean = sliders["Noise Mean"].val
    n_cov = sliders["Noise Covariance"].val
    cutoff = sliders["Cutoff Frequency"].val

    clean = harmonic_with_noise(amp, freq, phase, n_mean, n_cov, False)
    noisy = harmonic_with_noise(amp, freq, phase, n_mean, n_cov, True)
    filtered = apply_filter(noisy, cutoff)

    line_clean.set_ydata(clean)
    line_noisy.set_ydata(noisy)
    line_filtered.set_ydata(filtered)

    line_noisy.set_visible(show_noise)
    line_filtered.set_visible(show_filtered)

    ydata = clean
    if show_noise:
        ydata = noisy
    margin = 0.5
    ax.set_ylim(np.min(ydata) - margin, np.max(ydata) + margin)

    fig.canvas.draw_idle()


def on_noise_slider_changed(_):
    global noise
    n_mean = sliders["Noise Mean"].val
    n_cov = sliders["Noise Covariance"].val
    noise = np.random.normal(n_mean, np.sqrt(n_cov), len(t))
    update()


def on_check(label):
    global show_noise, show_filtered
    if label == "Show Noise":
        show_noise = not show_noise
    elif label == "Show Filtered":
        show_filtered = not show_filtered
    update()


def on_reset(_):
    global noise
    for name, _, _, vinit in slider_specs:
        sliders[name].set_val(vinit)
    noise = np.random.normal(DEFAULTS["noise_mean"],
                             np.sqrt(DEFAULTS["noise_covariance"]),
                             len(t))
    update()


# Connect harmonic sliders (don't regenerate noise)
for name in ("Amplitude", "Frequency", "Phase", "Cutoff Frequency"):
    sliders[name].on_changed(update)

# Connect noise sliders (regenerate noise)
for name in ("Noise Mean", "Noise Covariance"):
    sliders[name].on_changed(on_noise_slider_changed)

check.on_clicked(on_check)
btn_reset.on_clicked(on_reset)

# --- Instructions ---
print("=== Візуалізація гармоніки з шумом та фільтрацією ===")
print("y(t) = A * sin(2π * freq * t + phase) + noise")
print()
print("Керування:")
print("  Слайдери — змінюють параметри гармоніки, шуму та фільтра")
print("  Show Noise — показати/сховати зашумлений сигнал")
print("  Show Filtered — показати/сховати відфільтрований сигнал")
print("  Reset — відновити початкові параметри")
print()
print("Фільтр: Butterworth low-pass (порядок 4)")

plt.show()
