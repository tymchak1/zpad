import os
import re
import glob
import urllib.request
import streamlit as st
import pandas as pd
import numpy as np
import plotly.express as px
from datetime import datetime
from io import StringIO

# ── Mappings (from lab2) ──────────────────────────────────────────────

NOAA_TO_NAME = {
    1: "Черкаська", 2: "Чернігівська", 3: "Чернівецька",
    4: "АР Крим", 5: "Дніпропетровська", 6: "Донецька",
    7: "Івано-Франківська", 8: "Харківська", 9: "Херсонська",
    10: "Хмельницька", 11: "Київська", 12: "м. Київ",
    13: "Кіровоградська", 14: "Луганська", 15: "Львівська",
    16: "Миколаївська", 17: "Одеська", 18: "Полтавська",
    19: "Рівненська", 20: "м. Севастополь", 21: "Сумська",
    22: "Тернопільська", 23: "Закарпатська", 24: "Вінницька",
    25: "Волинська", 26: "Запорізька", 27: "Житомирська",
}

UA_ORDER = [24, 25, 5, 6, 27, 23, 26, 7, 11, 13, 14, 15, 16, 17, 18,
            19, 21, 22, 8, 9, 10, 1, 3, 2, 4, 12, 20]

NOAA_TO_UA = {}
for _ua_idx, _noaa_id in enumerate(UA_ORDER, 1):
    NOAA_TO_UA[_noaa_id] = (_ua_idx, NOAA_TO_NAME[_noaa_id])

# ── Data loading ──────────────────────────────────────────────────────

DATA_DIR = os.path.join(os.path.dirname(__file__), "vhi_data")


def download_vhi_data():
    os.makedirs(DATA_DIR, exist_ok=True)
    base_url = (
        "https://www.star.nesdis.noaa.gov/smcd/emb/vci/VH/"
        "get_TS_admin.php?country=UKR&provinceID={pid}"
        "&year1=1981&year2=2024&type=Mean"
    )
    now = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    for pid in range(1, 28):
        existing = glob.glob(os.path.join(DATA_DIR, f"vhi_id_{pid}_*.csv"))
        if existing:
            continue
        url = base_url.format(pid=pid)
        filepath = os.path.join(DATA_DIR, f"vhi_id_{pid}_{now}.csv")
        try:
            urllib.request.urlretrieve(url, filepath)
        except Exception:
            pass


def read_vhi_file(filepath, noaa_id):
    with open(filepath, "r") as f:
        text = f.read()
    text = re.sub(r"<[^>]+>", "", text)
    lines = text.strip().split("\n")
    header_idx = None
    for i, line in enumerate(lines):
        if re.match(r"\s*year[,\s]+week", line, re.IGNORECASE):
            header_idx = i
            break
    if header_idx is None:
        return pd.DataFrame()
    data_lines = [lines[header_idx].strip().rstrip(",")]
    for line in lines[header_idx + 1:]:
        stripped = line.strip()
        if stripped and not stripped.startswith("<") and re.match(r"\s*\d", stripped):
            data_lines.append(stripped.rstrip(","))
    try:
        df = pd.read_csv(StringIO("\n".join(data_lines)), sep=r"\s*,\s*",
                         engine="python", na_values=["-1", "-1.00"])
    except Exception:
        return pd.DataFrame()
    df = df.loc[:, ~df.columns.str.startswith("Unnamed")]
    df = df.drop(columns=["SMN", "SMT"], errors="ignore")
    for col in ["year", "week"]:
        df[col] = pd.to_numeric(df[col], errors="coerce")
    for col in ["VCI", "TCI", "VHI"]:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors="coerce")
    df = df.dropna(subset=["year", "week"])
    df["year"] = df["year"].astype(int)
    df["week"] = df["week"].astype(int)
    ua_idx, ua_name = NOAA_TO_UA[noaa_id]
    df["ua_index"] = ua_idx
    df["oblast"] = ua_name
    return df


@st.cache_data(show_spinner="Завантаження даних VHI…")
def load_all_vhi():
    download_vhi_data()
    frames = []
    for pid in range(1, 28):
        files = sorted(glob.glob(os.path.join(DATA_DIR, f"vhi_id_{pid}_*.csv")))
        if not files:
            continue
        df = read_vhi_file(files[-1], pid)
        if not df.empty:
            frames.append(df)
    if not frames:
        st.error("Не вдалося завантажити дані VHI.")
        st.stop()
    result = pd.concat(frames, ignore_index=True)
    result = result.sort_values(["ua_index", "year", "week"]).reset_index(drop=True)
    for col in ["VCI", "TCI", "VHI"]:
        result[col] = result.groupby("ua_index")[col].transform(
            lambda s: s.interpolate(method="linear", limit_direction="both")
        )
    return result


# ── App ───────────────────────────────────────────────────────────────

st.set_page_config(page_title="ЗПАД — Лаб 5", layout="wide")
st.title("Аналіз VHI даних по областях України")

df = load_all_vhi()

oblasts = (
    df[["ua_index", "oblast"]]
    .drop_duplicates()
    .sort_values("ua_index")
)
oblast_names = oblasts["oblast"].tolist()

year_min, year_max = int(df["year"].min()), int(df["year"].max())
week_min, week_max = int(df["week"].min()), int(df["week"].max())

# ── Sidebar (controls) ───────────────────────────────────────────────

with st.sidebar:
    st.header("Фільтри")

    index_col = st.selectbox("Часовий ряд", ["VCI", "TCI", "VHI"])

    selected_oblast = st.selectbox("Область", oblast_names)

    year_range = st.slider("Роки", year_min, year_max, (year_min, year_max))

    week_range = st.slider("Тижні", week_min, week_max, (week_min, week_max))

    sort_asc = st.checkbox("Сортувати за зростанням")
    sort_desc = st.checkbox("Сортувати за спаданням")

    if st.button("Reset"):
        st.rerun()

# ── Filter data ───────────────────────────────────────────────────────

mask = (
    (df["oblast"] == selected_oblast)
    & (df["year"] >= year_range[0])
    & (df["year"] <= year_range[1])
    & (df["week"] >= week_range[0])
    & (df["week"] <= week_range[1])
)
filtered = df.loc[mask].copy()

if sort_asc and sort_desc:
    st.sidebar.warning("Обрано обидва варіанти сортування — застосовано за зростанням.")
    filtered = filtered.sort_values(index_col, ascending=True)
elif sort_asc:
    filtered = filtered.sort_values(index_col, ascending=True)
elif sort_desc:
    filtered = filtered.sort_values(index_col, ascending=False)

# ── Tabs ──────────────────────────────────────────────────────────────

tab_table, tab_chart, tab_compare = st.tabs(
    ["Таблиця", "Графік", "Порівняння по областях"]
)

with tab_table:
    st.subheader(f"{index_col} — {selected_oblast}")
    st.dataframe(
        filtered[["year", "week", "VCI", "TCI", "VHI"]],
        use_container_width=True,
        hide_index=True,
    )

with tab_chart:
    st.subheader(f"{index_col} — {selected_oblast} ({year_range[0]}–{year_range[1]})")
    if filtered.empty:
        st.info("Немає даних для обраних фільтрів.")
    else:
        chart_df = filtered.sort_values(["year", "week"]).copy()
        chart_df["year_week"] = chart_df["year"].astype(str) + "-W" + chart_df["week"].astype(str).str.zfill(2)
        fig = px.line(chart_df, x="year_week", y=index_col,
                      title=f"{index_col} по тижнях",
                      labels={"year_week": "Рік-Тиждень", index_col: index_col})
        fig.update_xaxes(dtick=52, tickangle=45)
        st.plotly_chart(fig, use_container_width=True)

with tab_compare:
    st.subheader(f"Порівняння {index_col}: {selected_oblast} vs інші області")
    compare_mask = (
        (df["year"] >= year_range[0])
        & (df["year"] <= year_range[1])
        & (df["week"] >= week_range[0])
        & (df["week"] <= week_range[1])
    )
    compare_df = df.loc[compare_mask].copy()
    if compare_df.empty:
        st.info("Немає даних для обраних фільтрів.")
    else:
        avg = (
            compare_df.groupby("oblast")[index_col]
            .mean()
            .reset_index()
            .sort_values(index_col, ascending=False)
        )
        avg["highlight"] = avg["oblast"] == selected_oblast
        fig = px.bar(
            avg, x="oblast", y=index_col, color="highlight",
            color_discrete_map={True: "#EF553B", False: "#636EFA"},
            title=f"Середній {index_col} по областях",
            labels={"oblast": "Область", index_col: f"Середній {index_col}"},
        )
        fig.update_layout(showlegend=False, xaxis_tickangle=45)
        st.plotly_chart(fig, use_container_width=True)
