# BankVault — CLI Bank Management System (C)

> **CS50x Final Project**
> BankVault is a console-based bank management system written in **C**.
> It provides a realistic set of banking operations (accounts, deposits, withdrawals, transfers) with **persistent storage** and an **audit-style transaction ledger**.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Key Features](#key-features)
- [How It Works](#how-it-works)
  - [Data Model](#data-model)
  - [Persistence (Saving & Loading)](#persistence-saving--loading)
  - [Transaction Ledger](#transaction-ledger)
  - [Daily Withdrawal Limit](#daily-withdrawal-limit)
  - [Admin Login](#admin-login)
- [Project Structure](#project-structure)
- [Build & Run](#build--run)
  - [Dependencies](#dependencies)
  - [Compile](#compile)
  - [Run](#run)
- [How to Use](#how-to-use)
  - [Main Menu](#main-menu)
  - [Typical Scenarios](#typical-scenarios)
- [Design Decisions](#design-decisions)
- [Security Notes](#security-notes)
- [Limitations](#limitations)
- [Future Improvements](#future-improvements)
- [Testing Notes](#testing-notes)
- [Credits](#credits)

---

## Project Overview

**BankVault** simulates a simple banking back-office tool operated by an administrator.
It was designed as a CS50x Final Project to demonstrate:

- **Procedural programming in C** with clear modular structure
- **Data structures** (`struct`, `enum`) to represent real entities (Accounts & Transactions)
- **File I/O** (binary persistence) so data survives program restarts
- **Validation** and **edge case handling** (duplicate accounts, invalid inputs, insufficient funds, daily limits)
- A minimal **authentication layer** for admin access
- A **transaction log** that acts like a bank ledger (audit trail)

This project is intentionally terminal-based: it focuses on core programming fundamentals, correctness, and data integrity rather than graphical UI.

---

## Key Features

### ✅ Accounts Management (CRUD)

- Create accounts with:
  - Unique account number (auto-generated or manual)
  - Customer name & phone number
  - Account type: **SAVINGS** or **CURRENT**
  - Initial deposit
- View account details (formatted “account card”)
- Edit account details (name/phone)
- Soft delete accounts (keeps historical data and transaction trace)

### ✅ Banking Operations

- Deposit money
- Withdraw money (with validations)
- Transfer money between accounts

### ✅ Transaction Ledger (Audit Trail)

Every important operation is stored in a separate transaction log:

- Account creation
- Edit
- Delete
- Deposit
- Withdraw
- Transfer in/out
- Interest application

This creates a reliable “history” of actions, similar to real bank logging systems.

### ✅ Statement Generation

- Generate a statement showing the **last N transactions** for a specific account.

### ✅ Dashboard / Statistics

- Total active accounts
- Total balance held by the bank (active accounts only)
- Breakdown by account type (savings/current)

### ✅ Monthly Interest Simulation (Savings Accounts)

- Apply monthly interest to savings accounts using a user-entered rate.

### ✅ Export to CSV

- Export accounts to a CSV file for external reporting.

### ✅ Persistent Storage

- Accounts and transactions are saved to files and reloaded automatically when the program restarts.

---

## How It Works

### Data Model

BankVault is built around two core entities:

#### 1) Account

An account is stored as a C `struct` containing:

- `accountNumber` (unique key)
- `name`, `phone`
- `type` (Savings / Current)
- `balance`
- `isActive` (soft delete flag)
- `createdAt` and `updatedAt` timestamps
- Daily withdrawal tracking fields:
  - `dailyWithdrawn`
  - last withdrawal date fields

This design mimics a small database record inside a binary file.

#### 2) Transaction

A transaction is recorded whenever money moves or data changes:

- `txId` (auto-increment ID)
- `accountNumber`
- `type` (Deposit/Withdraw/Transfer/etc.)
- `amount`
- `relatedAccount` (used for transfers)
- `balanceAfter`
- timestamp
- small text note

---

### Persistence (Saving & Loading)

To ensure the program behaves like a real system, BankVault writes to disk:

#### Accounts storage (`accounts.bin`)

- The program saves:
  1. `g_count` (how many accounts exist)
  2. the entire accounts array

On startup:

- it loads `g_count`
- reads all accounts into memory

This makes the program fast during runtime (in-memory operations), while remaining persistent across runs.

#### Transactions storage (`tx.bin`)

Transactions are appended to `tx.bin`:

- this makes it a true “ledger”
- it preserves historical record even if an account is deleted

On startup:

- the program scans the file to calculate the next `txId` (so IDs continue correctly).

---

### Transaction Ledger

BankVault follows a common real-world approach:

- **Accounts file** represents the “current state”
- **Transaction file** represents the “history”

That separation is important:

- you can rebuild history
- you can show statements
- you can audit actions

Example:

- When transferring money:
  - it records `TRANSFER_OUT` for sender
  - and `TRANSFER_IN` for receiver

This makes it very clear what happened and to whom.

---

### Daily Withdrawal Limit

Withdrawals (and outgoing transfers) are restricted by a daily limit:

- The system tracks how much was withdrawn today per account.
- If a new day begins, the counter resets automatically.
- This prevents unrealistic unlimited withdrawals.

This feature demonstrates:

- state tracking
- time-aware logic
- real-world constraints

---

### Admin Login

On startup, the system requires an admin login.

- Default credentials:
  - Username: `admin`
  - Password: `admin123`

Optionally, the user can create a file named `admin.cfg`:

- Line 1: username
- Line 2: password

This is a **simple educational approach** intended for a console project.
(Real systems require password hashing and better security; see [Security Notes](#security-notes).)

---

## Project Structure

BankVault is divided into modules to keep code clean and maintainable:

BankVault/
main.c # Application flow + menu actions
config.h # Constants and config values
utils.h / utils.c # Input handling + time helpers
accounts.h / accounts.c # Account types and helper functions
transactions.h / transactions.c # Transaction types and helpers
storage.h / storage.c # File I/O (load/save accounts, append tx)
ui.h / ui.c # Menu printing and account display formatting
README.md

Why this matters:

- modularity makes the project easier to understand
- each file has one responsibility
- this reflects good CS50 software engineering practice

---

## Build & Run

### Dependencies

You need:

- GCC or Clang compiler
- A terminal environment

### Compile

Inside the `BankVault` folder:

```bash
gcc main.c utils.c accounts.c transactions.c storage.c ui.c -o bankvault
Run
On macOS / Linux:

./bankvault
On Windows:

bankvault.exe
How to Use
Main Menu
Once you log in, you can choose from options like:

Dashboard

Create / View / Edit / Delete accounts

Deposit / Withdraw / Transfer

List accounts

Search by name

Print account statement

Apply interest

Export CSV

Typical Scenarios
Scenario 1: Create Account → Deposit → Withdraw

Create a savings account with an initial deposit

Deposit additional funds

Withdraw money (within daily limit)

Print statement for last 10 transactions

Scenario 2: Transfer Between Accounts

Create two accounts

Transfer money from account A to account B

Confirm:

Sender balance decreased

Receiver balance increased

Both accounts show transfer logs

Design Decisions
Why binary files instead of text?
Binary storage:

is faster to load/write for structs

preserves data types exactly (double/int)

is simpler for C-level persistence

Why “soft delete”?
Soft delete keeps history:

real banks rarely “erase” accounts completely

audit logs must remain

the system can still show that an account existed and was deleted

Why transaction log separated from account state?
This mirrors real design:

accounts = current state

transactions = immutable history

Security Notes
This project is educational and runs locally.
For real security, we would need:

hashed passwords (bcrypt/scrypt/argon2)

permission roles (admin vs teller vs customer)

encryption at rest

logging login attempts

input hardening against malicious usage

file locking for multi-process safety

BankVault focuses on correctness, structure, and features suitable for CS50.

Limitations
Single-user, local usage (not multi-user networked)

No encryption for files

No concurrency support (file locking not implemented)

CSV export is basic and may not escape all edge cases

Interest is manually applied (not scheduled)

Future Improvements
If extended further, possible upgrades:

Implement password hashing

Add “Undo last transaction”

Add account import from CSV

Add sorting/filtering by balance/type/status

Add better search tools (phone, type, active only)

Add roles and permissions

Add unit tests with scripted input and expected outputs

Testing Notes
This project was tested by:

Creating multiple accounts and verifying unique IDs

Deposits and withdrawals with:

invalid amounts

edge cases (0, negative, over-balance)

Transfer operations:

same account

missing target account

insufficient funds

Persistence:

restart program and verify accounts still exist

verify transaction IDs continue correctly

Daily withdrawal limit enforcement

Credits
Developed by: Adam Abamahdi
Course: CS50x
Language: C
Tools: VS Code, GCC/Clang
```
