import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import shutil
from datetime import datetime

# Define sensitive keywords
SENSITIVE_KEYS = ["PASSWORD", "SECRET", "API_KEY", "JWT_SECRET"]

class EnvEditor:
    def __init__(self, root):
        self.root = root
        self.root.title(".env Visual Manager PRO")
        self.root.geometry("800x450")

        self.env_path = None
        self.data = []

        self.create_ui()

    def create_ui(self):
        # Top frame
        top = tk.Frame(self.root)
        top.pack(fill="x", padx=10, pady=5)

        tk.Button(top, text="Open .env", command=self.open_env).pack(side="left")
        tk.Button(top, text="Save", command=self.save_env).pack(side="left", padx=5)
        tk.Button(top, text="Restore Backup", command=self.restore_backup).pack(side="left", padx=5)

        # Show secrets
        self.show_secrets_var = tk.BooleanVar()
        tk.Checkbutton(top, text="Show Secrets", variable=self.show_secrets_var, command=self.refresh_tree).pack(side="left", padx=5)

        # Search
        tk.Label(top, text="Search:").pack(side="left", padx=5)
        self.search_var = tk.StringVar()
        tk.Entry(top, textvariable=self.search_var).pack(side="left")
        tk.Button(top, text="Go", command=self.search_key).pack(side="left", padx=5)

        # Treeview
        self.tree = ttk.Treeview(
            self.root,
            columns=("key", "value"),
            show="headings"
        )
        self.tree.heading("key", text="KEY")
        self.tree.heading("value", text="VALUE")
        self.tree.pack(fill="both", expand=True, padx=10, pady=10)
        self.tree.bind("<Double-1>", self.edit_value)

    # Open .env file
    def open_env(self):
        path = filedialog.askopenfilename(filetypes=[(".env files", "*.env")])
        if not path:
            return

        self.env_path = path
        self.load_env()

    # Load data
    def load_env(self):
        self.tree.delete(*self.tree.get_children())
        self.data.clear()

        with open(self.env_path, "r", encoding="utf-8") as f:
            for line in f:
                line_strip = line.strip()
                if not line_strip or line_strip.startswith("#") or "=" not in line_strip:
                    continue

                key, value = line_strip.split("=", 1)
                self.data.append([key, value])

        self.refresh_tree()

    # Refresh table according to show secrets checkbox
    def refresh_tree(self):
        self.tree.delete(*self.tree.get_children())
        for key, value in self.data:
            if self.show_secrets_var.get():
                display_value = value
            else:
                display_value = "******" if any(s in key.upper() for s in SENSITIVE_KEYS) else value
            self.tree.insert("", "end", values=(key, display_value))
        self.highlight_issues()

    # Highlight duplicates and empty values
    def highlight_issues(self):
        keys_seen = {}
        for item in self.tree.get_children():
            key, value = self.tree.item(item)["values"]
            # duplicates
            if key in keys_seen:
                self.tree.item(item, tags=("duplicate",))
                self.tree.item(keys_seen[key], tags=("duplicate",))
            else:
                keys_seen[key] = item
            # empty
            if value == "" or value is None:
                self.tree.item(item, tags=("empty",))
        self.tree.tag_configure("duplicate", background="yellow")
        self.tree.tag_configure("empty", background="red")

    # Edit value in popup
    def edit_value(self, event):
        item = self.tree.selection()
        if not item:
            return
        item = item[0]
        key = self.tree.item(item)["values"][0]

        index = next(i for i, v in enumerate(self.data) if v[0] == key)
        old_value = self.data[index][1]

        popup = tk.Toplevel(self.root)
        popup.title(f"Edit: {key}")
        popup.geometry("300x120")

        tk.Label(popup, text=key).pack(pady=5)
        entry = tk.Entry(popup, show="*" if any(s in key.upper() for s in SENSITIVE_KEYS) else "")
        entry.insert(0, old_value)
        entry.pack()

        def save():
            new_value = entry.get()
            self.data[index][1] = new_value
            self.refresh_tree()
            popup.destroy()

        def copy_value():
            self.root.clipboard_clear()
            self.root.clipboard_append(old_value)
            messagebox.showinfo("Copied", "Value copied to clipboard")

        tk.Button(popup, text="Save", command=save).pack(side="left", padx=10, pady=10)
        tk.Button(popup, text="Copy", command=copy_value).pack(side="right", padx=10, pady=10)

    # Save .env with backup
    def save_env(self):
        if not self.env_path:
            messagebox.showwarning("Warning", "Please open a .env file first before saving.")
            return

        backup_name = self.env_path + ".backup_" + datetime.now().strftime("%Y%m%d%H%M%S")
        shutil.copy(self.env_path, backup_name)

        with open(self.env_path, "w", encoding="utf-8") as f:
            for key, value in self.data:
                f.write(f"{key}={value}\n")

        messagebox.showinfo("Saved", f"File saved successfully.\nBackup created:\n{backup_name}")
        self.refresh_tree()

    # Search key
    def search_key(self):
        query = self.search_var.get().strip().upper()
        if not query:
            return
        for item in self.tree.get_children():
            key_val = self.tree.item(item)["values"][0].upper()
            if query in key_val:
                self.tree.selection_set(item)
                self.tree.focus(item)
                self.tree.see(item)
                break

    # Restore backup
    def restore_backup(self):
        if not self.env_path:
            messagebox.showwarning("Warning", "Please open a .env file first before restoring a backup.")
            return

        path = filedialog.askopenfilename(filetypes=[("Backup files", "*.backup_*")])
        if not path:
            return

        shutil.copy(path, self.env_path)
        self.load_env()
        messagebox.showinfo("Restored", "Backup restored successfully")

if __name__ == "__main__":
    root = tk.Tk()
    app = EnvEditor(root)
    root.mainloop()
