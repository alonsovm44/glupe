# ------------------------------------------------------------
# Generated Python file from semantic transpilation request
# ------------------------------------------------------------

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import shutil
import datetime

# Constants
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
        top = tk.Frame(self.root)
        top.pack(fill="x")

        tk.Button(top, text="Open .env", command=self.open_env).pack(side="left")
        tk.Button(top, text="Save", command=self.save_env).pack(side="left")
        tk.Button(top, text="Restore Backup", command=self.restore_backup).pack(side="left")

        self.show_secrets_var = tk.BooleanVar(value=False)
        tk.Checkbutton(top, text="Show Secrets", variable=self.show_secrets_var,
                       command=self.refresh_tree).pack(side="left", padx=10)

        self.search_var = tk.StringVar()
        tk.Entry(top, textvariable=self.search_var, width=20).pack(side="left")
        tk.Button(top, text="Go", command=self.search_key).pack(side="left")

        self.tree = ttk.Treeview(self.root, columns=("key", "value"), show="headings")
        self.tree.heading("key", text="Key")
        self.tree.heading("value", text="Value")
        self.tree.pack(fill="both", expand=True)
        self.tree.bind("<Double-1>", self.edit_value)

        self.tree.tag_configure("duplicate", background="#ffcccc")
        self.tree.tag_configure("empty", background="#fff2cc")

    def open_env(self):
        path = filedialog.askopenfilename(title="Select .env file")
        if not path:
            return
        self.env_path = path
        self.load_env()

    def load_env(self):
        self.tree.delete(*self.tree.get_children())
        self.data = []

        try:
            with open(self.env_path, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if "=" in line:
                        key, value = line.split("=", 1)
                        self.data.append([key, value])
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load file: {e}")
            return

        self.refresh_tree()

    def refresh_tree(self):
        self.tree.delete(*self.tree.get_children())

        for key, value in self.data:
            display_value = value
            if not self.show_secrets_var.get():
                if any(s in key.upper() for s in SENSITIVE_KEYS):
                    display_value = "••••••••"
            self.tree.insert("", "end", values=(key, display_value))

        self.highlight_issues()

    def highlight_issues(self):
        keys_seen = {}
        for item in self.tree.get_children():
            key, value = self.tree.item(item, "values")

            tags = []

            if key in keys_seen:
                tags.append("duplicate")
            else:
                keys_seen[key] = True

            if value.strip() == "":
                tags.append("empty")

            if tags:
                self.tree.item(item, tags=tags)

    def edit_value(self, event):
        selected = self.tree.selection()
        if not selected:
            return

        item = selected[0]
        key, old_value = self.tree.item(item, "values")

        index = None
        for i, (k, v) in enumerate(self.data):
            if k == key:
                index = i
                break
        if index is None:
            return

        popup = tk.Toplevel(self.root)
        popup.title(f"Edit Value: {key}")

        tk.Label(popup, text=f"Key: {key}").pack(pady=5)
        val_var = tk.StringVar(value=old_value)
        tk.Entry(popup, textvariable=val_var, width=40).pack(pady=5)

        def save():
            self.data[index][1] = val_var.get()
            popup.destroy()
            self.refresh_tree()

        def copy_value():
            self.root.clipboard_clear()
            self.root.clipboard_append(old_value)

        tk.Button(popup, text="Save", command=save).pack(side="left", padx=10, pady=10)
        tk.Button(popup, text="Copy Old Value", command=copy_value).pack(side="left", padx=10)

    def save_env(self):
        if not self.env_path:
            messagebox.showwarning("Warning", "No .env file opened.")
            return

        backup_path = self.env_path + ".backup_" + datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        shutil.copy2(self.env_path, backup_path)

        try:
            with open(self.env_path, "w", encoding="utf-8") as f:
                for key, value in self.data:
                    f.write(f"{key}={value}\n")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save file: {e}")
            return

        messagebox.showinfo("Saved", "Environment file saved successfully.")
        self.refresh_tree()

    def search_key(self):
        query = self.search_var.get().strip()
        if not query:
            return

        for item in self.tree.get_children():
            key, _ = self.tree.item(item, "values")
            if query.lower() in key.lower():
                self.tree.selection_set(item)
                self.tree.see(item)
                return

        messagebox.showinfo("Not Found", "No matching key found.")

    def restore_backup(self):
        if not self.env_path:
            messagebox.showwarning("Warning", "No .env file opened.")
            return

        backup_path = filedialog.askopenfilename(title="Select Backup File")
        if not backup_path:
            return

        try:
            shutil.copy2(backup_path, self.env_path)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to restore backup: {e}")
            return

        self.load_env()
        messagebox.showinfo("Restored", "Backup restored successfully.")


# Main
if __name__ == "__main__":
    root = tk.Tk()
    app = EnvEditor(root)
    root.mainloop()
