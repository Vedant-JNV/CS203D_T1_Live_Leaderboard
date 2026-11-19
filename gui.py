import tkinter as tk
from tkinter import ttk, messagebox
import subprocess, threading, time, csv, os

ROOT = os.path.dirname(os.path.abspath(__file__))
APP = os.path.join(ROOT, "run.exe")
TEST = os.path.join(ROOT, "difftest.exe")
BENCH = os.path.join(ROOT, "bench", "run_benchmarks.bat")

BG = "#10151f"
PANEL = "#1a2230"
FG = "#e5eaf2"
ACCENT = "#4da3ff"

class LeaderboardGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Live Leaderboard | AVL vs BST")
        self.root.geometry("1100x720")
        self.root.minsize(900, 600)
        self.root.configure(bg=BG)

        self.history = []
        self.players = {}
        self.structure = tk.StringVar(value="A")
        self.busy = False

        self.setup_style()
        self.build_ui()
        self.refresh_table()

    def setup_style(self):
        style = ttk.Style()
        style.theme_use("clam")
        style.configure(".", font=("Segoe UI", 10))
        style.configure("TFrame", background=BG)
        style.configure("Panel.TFrame", background=PANEL)
        style.configure("TLabel", background=BG, foreground=FG)
        style.configure("Panel.TLabel", background=PANEL, foreground=FG)
        style.configure("TButton", padding=7)
        style.configure("Accent.TButton", background=ACCENT, foreground="white")
        style.configure("TNotebook", background=BG, borderwidth=0)
        style.configure("TNotebook.Tab", padding=(18, 9))
        style.configure("Treeview", background=PANEL, foreground=FG,
                       fieldbackground=PANEL, rowheight=28)
        style.configure("Treeview.Heading", background="#273448",
                       foreground=FG, font=("Segoe UI", 10, "bold"))
        style.map("Treeview", background=[("selected", "#294d77")])

    def build_ui(self):
        header = tk.Frame(self.root, bg=BG)
        header.pack(fill="x", padx=22, pady=(18, 12))

        tk.Label(header, text="LIVE LEADERBOARD", bg=BG, fg=FG,
                 font=("Segoe UI", 22, "bold")).pack(side="left")
        tk.Label(header, text="AVL  vs  BST", bg=BG, fg=ACCENT,
                 font=("Segoe UI", 12, "bold")).pack(side="left", padx=18)

        self.tabs = ttk.Notebook(self.root)
        self.tabs.pack(fill="both", expand=True, padx=20, pady=(0, 20))

        self.demo_tab = ttk.Frame(self.tabs)
        self.test_tab = ttk.Frame(self.tabs)
        self.bench_tab = ttk.Frame(self.tabs)

        self.tabs.add(self.demo_tab, text="  Live Demo  ")
        self.tabs.add(self.test_tab, text="  Testing  ")
        self.tabs.add(self.bench_tab, text="  Benchmark  ")

        self.build_demo()
        self.build_testing()
        self.build_benchmark()

    # ---------- LIVE DEMO ----------

    def build_demo(self):
        top = ttk.Frame(self.demo_tab)
        top.pack(fill="x", padx=15, pady=12)

        ttk.Label(top, text="Structure:").pack(side="left")
        ttk.Radiobutton(top, text="AVL Tree (A)", variable=self.structure,
                        value="A", command=self.refresh_table).pack(side="left", padx=8)
        ttk.Radiobutton(top, text="BST (B)", variable=self.structure,
                        value="B", command=self.refresh_table).pack(side="left", padx=8)

        body = ttk.Frame(self.demo_tab)
        body.pack(fill="both", expand=True, padx=15, pady=5)

        left = ttk.Frame(body, style="Panel.TFrame", padding=15)
        left.pack(side="left", fill="y", padx=(0, 12))

        ttk.Label(left, text="PLAYER OPERATIONS",
                  style="Panel.TLabel").pack(anchor="w", pady=(0, 12))

        self.id_entry = self.make_entry(left, "Player ID")
        self.score_entry = self.make_entry(left, "Score")

        ttk.Button(left, text="Add / Update Player",
                   command=self.update_player).pack(fill="x", pady=5)
        ttk.Button(left, text="Delete Player",
                   command=self.delete_player).pack(fill="x", pady=5)

        ttk.Separator(left).pack(fill="x", pady=12)

        self.rank_entry = self.make_entry(left, "Rank: Player ID")
        ttk.Button(left, text="Find Rank",
                   command=self.find_rank).pack(fill="x", pady=4)

        self.select_entry = self.make_entry(left, "Select K")
        ttk.Button(left, text="Select Player",
                   command=self.select_player).pack(fill="x", pady=4)

        self.range_a = self.make_entry(left, "Range Start")
        self.range_b = self.make_entry(left, "Range End")
        ttk.Button(left, text="Get Rank Range",
                   command=self.range_query).pack(fill="x", pady=4)

        ttk.Button(left, text="Clear Demo",
                   command=self.clear_demo).pack(fill="x", pady=(18, 4))

        right = ttk.Frame(body)
        right.pack(side="left", fill="both", expand=True)

        self.stats = tk.Label(right, text="", bg=BG, fg=ACCENT,
                              font=("Segoe UI", 12, "bold"))
        self.stats.pack(anchor="w", pady=(0, 10))

        self.tree = ttk.Treeview(right, columns=("rank", "id", "score"),
                                 show="headings")
        self.tree.heading("rank", text="Rank")
        self.tree.heading("id", text="Player ID")
        self.tree.heading("score", text="Score")
        self.tree.column("rank", width=90, anchor="center")
        self.tree.column("id", width=180, anchor="center")
        self.tree.column("score", width=180, anchor="center")
        self.tree.pack(fill="both", expand=True)

        self.demo_output = tk.Label(right, text="Ready.",
                                   bg=PANEL, fg=FG, anchor="w",
                                   padx=10, pady=10)
        self.demo_output.pack(fill="x", pady=(10, 0))

    def make_entry(self, parent, placeholder):
        e = ttk.Entry(parent, width=22)
        e.pack(fill="x", pady=4)
        e.insert(0, placeholder)
        e.bind("<FocusIn>", lambda event: self.clear_placeholder(e, placeholder))
        return e

    @staticmethod
    def clear_placeholder(entry, placeholder):
        if entry.get() == placeholder:
            entry.delete(0, "end")

    def get_int(self, entry, label):
        try:
            return int(entry.get())
        except ValueError:
            messagebox.showerror("Invalid input", f"{label} must be an integer.")
            return None

    def cpp(self, operations):
        """Send operations to the actual compiled C++ program."""
        data = "\n".join(operations) + "\n"
        result = subprocess.run(
            [APP, "--structure", self.structure.get()],
            input=data, text=True, capture_output=True, cwd=ROOT
        )
        if result.returncode != 0:
            raise RuntimeError(result.stderr or "C++ application failed")
        return result.stdout.strip().splitlines()

    def replay(self, extra=None):
        ops = self.history.copy()
        if extra:
            ops.extend(extra)
        return self.cpp(ops)

    def update_player(self):
        pid = self.get_int(self.id_entry, "Player ID")
        score = self.get_int(self.score_entry, "Score")
        if pid is None or score is None:
            return
        self.history.append(f"U {pid} {score}")
        self.players[pid] = score
        self.refresh_table()
        self.demo_output.config(text=f"Updated Player {pid} → Score {score}")

    def delete_player(self):
        pid = self.get_int(self.id_entry, "Player ID")
        if pid is None:
            return
        self.history.append(f"D {pid}")
        self.players.pop(pid, None)
        self.refresh_table()
        self.demo_output.config(text=f"Deleted Player {pid}")

    def refresh_table(self):
        for row in self.tree.get_children():
            self.tree.delete(row)

        # Sorting here is display-only; query outputs still come from C++.
        ordered = sorted(self.players.items(), key=lambda p: (-p[1], p[0]))
        self.stats.config(text=f"{len(ordered)} players  |  Structure {self.structure.get()}")

        for i, (pid, score) in enumerate(ordered, 1):
            rank = i
            if i > 1 and ordered[i-2][1] == score:
                rank = int(self.tree.item(self.tree.get_children()[-1])["values"][0])
            self.tree.insert("", "end", values=(rank, pid, score))

    def find_rank(self):
        pid = self.get_int(self.rank_entry, "Player ID")
        if pid is None:
            return
        out = self.replay([f"R {pid}"])
        self.demo_output.config(text=f"Rank of {pid}: {out[-1] if out else 'No output'}")

    def select_player(self):
        k = self.get_int(self.select_entry, "K")
        if k is None:
            return
        out = self.replay([f"S {k}"])
        self.demo_output.config(text=f"Player at rank {k}: {out[-1] if out else 'No output'}")

    def range_query(self):
        a = self.get_int(self.range_a, "Range Start")
        b = self.get_int(self.range_b, "Range End")
        if a is None or b is None:
            return
        out = self.replay([f"L {a} {b}"])
        self.demo_output.config(text=f"Range [{a}, {b}]: {' '.join(out)}")

    def clear_demo(self):
        self.history.clear()
        self.players.clear()
        self.refresh_table()
        self.demo_output.config(text="Demo cleared.")

    # ---------- TESTING ----------

    def build_testing(self):
        frame = ttk.Frame(self.test_tab, padding=15)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="Differential & Invariant Tests",
                 font=("Segoe UI", 16, "bold")).pack(anchor="w")
        ttk.Label(frame, text="Runs the existing C++ test executable.",
                 ).pack(anchor="w", pady=(4, 12))

        self.test_button = ttk.Button(frame, text="▶  Run Tests",
                                     command=self.run_tests)
        self.test_button.pack(anchor="w")

        self.test_status = tk.Label(frame, text="Ready", bg=BG, fg=ACCENT,
                                   font=("Segoe UI", 11, "bold"))
        self.test_status.pack(anchor="w", pady=10)

        self.test_log = self.make_log(frame)

    def make_log(self, parent):
        text = tk.Text(parent, bg="#0b1018", fg="#cbd5e1",
                      insertbackground=FG, font=("Consolas", 10),
                      wrap="none", relief="flat")
        text.pack(fill="both", expand=True, pady=5)
        return text

    def run_tests(self):
        if self.busy:
            return
        if not os.path.exists(TEST):
            messagebox.showerror("Missing executable",
                               "difftest.exe not found in project root.")
            return
        self.busy = True
        self.test_button.config(state="disabled")
        self.test_status.config(text="Running...", fg="#fbbf24")
        self.test_log.delete("1.0", "end")
        threading.Thread(target=self.test_worker, daemon=True).start()

    def test_worker(self):
        start = time.perf_counter()
        try:
            result = subprocess.run(
                [TEST], capture_output=True, text=True, cwd=ROOT, timeout=600
            )
            elapsed = time.perf_counter() - start
            output = result.stdout + result.stderr
            code = result.returncode
            self.root.after(
                0, lambda c=code, out=output, t=elapsed:
                    self.finish_test(c, out, t)
            )
        except Exception as e:
            error = str(e)
            elapsed = time.perf_counter() - start
            self.root.after(
                0, lambda msg=error, t=elapsed:
                    self.finish_test(-1, msg, t)
            )

    def finish_test(self, code, output, elapsed):
        self.test_log.delete("1.0", "end")
        self.test_log.insert("end", output if output.strip() else "(No output returned.)")
        passed = code == 0
        self.test_status.config(
            text=f"{'PASS' if passed else 'FAIL'} | Exit code: {code} | {elapsed:.2f}s",
            fg="#4ade80" if passed else "#f87171"
        )
        self.test_button.config(state="normal")
        self.busy = False

    # ---------- BENCHMARK ----------

    def build_benchmark(self):
        frame = ttk.Frame(self.bench_tab, padding=12)
        frame.pack(fill="both", expand=True)

        ttk.Label(frame, text="Performance Benchmark",
                font=("Segoe UI", 16, "bold")).pack(anchor="w")

        controls = ttk.Frame(frame)
        controls.pack(fill="x", pady=6)

        self.bench_button = ttk.Button(
            controls, text="▶ Run Benchmark", command=self.run_bench
        )
        self.bench_button.pack(side="left")

        ttk.Button(controls, text="Load Existing Results",
                command=self.plot_existing).pack(side="left", padx=8)

        self.bench_status = tk.Label(
            frame, text="Ready", bg=BG, fg=ACCENT,
            font=("Segoe UI", 11, "bold")
        )
        self.bench_status.pack(anchor="w", pady=4)

        self.bench_log = tk.Text(
            frame, height=5, bg="#0b1018", fg="#cbd5e1",
            font=("Consolas", 9), wrap="word"
        )
        self.bench_log.pack(fill="x", pady=5)

        # Scrollable graph dashboard
        self.chart_canvas = tk.Canvas(frame, bg=BG, highlightthickness=0)
        scrollbar = ttk.Scrollbar(frame, orient="vertical",
                                command=self.chart_canvas.yview)
        self.chart_canvas.configure(yscrollcommand=scrollbar.set)

        scrollbar.pack(side="right", fill="y")
        self.chart_canvas.pack(side="left", fill="both", expand=True)

        self.chart_dashboard = ttk.Frame(self.chart_canvas)
        self.dashboard_window = self.chart_canvas.create_window(
            (0, 0), window=self.chart_dashboard, anchor="nw"
        )

        self.chart_dashboard.bind(
            "<Configure>",
            lambda e: self.chart_canvas.configure(
                scrollregion=self.chart_canvas.bbox("all")
            )
        )

        self.chart_canvas.bind(
            "<Configure>",
            lambda e: self.chart_canvas.itemconfigure(
                self.dashboard_window, width=e.width
            )
        )

        # Mouse wheel scrolling
        self.chart_canvas.bind("<MouseWheel>",
            lambda e: self.chart_canvas.yview_scroll(
                int(-1 * (e.delta / 120)), "units"
            )
        )

    def run_bench(self):
        if self.busy:
            return
        if not os.path.exists(BENCH):
            messagebox.showerror("Missing script", "Benchmark .bat file not found.")
            return
        self.busy = True
        self.bench_button.config(state="disabled")
        self.bench_status.config(text="Benchmark running...", fg="#fbbf24")
        self.bench_log.delete("1.0", "end")
        threading.Thread(target=self.bench_worker, daemon=True).start()

    def bench_worker(self):
        try:
            result = subprocess.run(
                ["cmd", "/c", BENCH], capture_output=True, text=True,
                cwd=ROOT, timeout=3600
            )
            output = result.stdout + result.stderr
            code = result.returncode
            self.root.after(0, lambda c=code, out=output: self.finish_bench(c, out))
        except Exception as e:
            error = str(e)
            self.root.after(0, lambda msg=error: self.finish_bench(-1, msg))

    def finish_bench(self, code, output):
        self.bench_log.insert("end", output)
        self.bench_status.config(
            text="Finished" if code == 0 else "Failed",
            fg="#4ade80" if code == 0 else "#f87171"
        )
        self.bench_button.config(state="normal")
        self.busy = False
        if code == 0:
            self.plot_existing()

    def plot_existing(self):
        import csv
        from collections import defaultdict
        import statistics
        from matplotlib.figure import Figure
        from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

        csv_path = os.path.join(ROOT, "results", "benchmark_raw.csv")
        if not os.path.exists(csv_path):
            messagebox.showinfo("No results", "benchmark_raw.csv not found.")
            return

        try:
            with open(csv_path, newline="", encoding="utf-8") as f:
                rows = list(csv.DictReader(f))
            if not rows:
                messagebox.showinfo("Empty results", "The CSV contains no benchmark data.")
                return

            groups = defaultdict(list)
            for row in rows:
                groups[(row["structure"], row["arrival"], int(row["n"]))].append(row)

            metrics = [
                ("insertion_ns_per_op", "Insertion Time (ns/op)"),
                ("rank_ns_per_op", "Rank Query (ns/op)"),
                ("select_ns_per_op", "Select Query (ns/op)"),
                ("height", "Tree Height"),
                ("rotations_per_insert", "AVL Rotations per Insert"),
            ]

            summary = defaultdict(dict)
            for (structure, arrival, n), group in groups.items():
                for metric, _ in metrics:
                    values = [float(r[metric]) for r in group if r.get(metric, "").strip()]
                    if values:
                        summary[(structure, arrival, n)][metric] = statistics.mean(values)

            for widget in self.chart_dashboard.winfo_children():
                widget.destroy()

            chart_count = 0
            for metric, ylabel in metrics:
                for arrival in ("random", "sorted"):
                    structures = ("AVL",) if metric == "rotations_per_insert" else ("AVL", "BST")
                    fig = Figure(figsize=(5.2, 3.35), dpi=100)
                    ax = fig.add_subplot(111)

                    for structure in structures:
                        subset = sorted(
                            [(n, vals[metric]) for (s, a, n), vals in summary.items()
                             if s == structure and a == arrival and metric in vals],
                            key=lambda pair: pair[0]
                        )
                        if subset:
                            xs, ys = zip(*subset)
                            ax.plot(xs, ys, marker="o", linewidth=2, markersize=4,
                                    label=structure)

                    ax.set_xscale("log")
                    if metric not in ("height", "rotations_per_insert"):
                        ax.set_yscale("log")
                    ax.set_xlabel("Number of Players")
                    ax.set_ylabel(ylabel)
                    ax.set_title(f"{ylabel} — {arrival.title()} Arrival")
                    ax.grid(True, which="both", alpha=0.25)
                    if ax.lines:
                        ax.legend()
                    fig.tight_layout()

                    chart = FigureCanvasTkAgg(fig, master=self.chart_dashboard)
                    chart.draw()
                    chart.get_tk_widget().grid(
                        row=chart_count // 2, column=chart_count % 2,
                        padx=8, pady=8, sticky="nsew"
                    )
                    chart_count += 1

            self.chart_dashboard.grid_columnconfigure(0, weight=1)
            self.chart_dashboard.grid_columnconfigure(1, weight=1)
            self.chart_canvas.configure(scrollregion=self.chart_canvas.bbox("all"))
            self.bench_status.config(
                text=f"Loaded {chart_count} benchmark plots", fg="#4ade80"
            )
        except Exception as e:
            messagebox.showerror("Plot Error", str(e))

if __name__ == "__main__":
    root = tk.Tk()
    app = LeaderboardGUI(root)
    root.mainloop()