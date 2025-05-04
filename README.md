# MOE ~ *Memory Oriented languagE* ~

memory management is one of the most critical aspects of system programming languages.

traditional languages like *C* and *C++* offer fine-grained control but leave developers responsible
for preventing memory leaks, dangling references, and undefined behavior. meanwhile, *Rust* introduces
strict ownership rules to ensure safety, but its borrow checker can be complex and difficult to work with.

this project proposes a language design that aims to balances **safety, efficiency, and ease of use**.

## 1. Pass-by-Value Semantics

all function arguments in *MOE* are passed by value,
regardless of whether they are primitives, structs, or ptr.

this simplifies reasoning and eliminates the need for complex
lvalue/rvalue references, and implicit borrowing or hidden moves.

## 2. Ownership and Memory Model

*MOE* introduces a strict distinction between **stack** or **static** and **heap**,
making it easier to reason about memory safety and object lifetimes in daily basis.

| Type     | For                     | Safe?  |
|--------- |------------------------ |------- |
| `ptr<T>` | **heap** objects        | ⚠️ No  |
| `ref<T>` | **stack** objects       | ✅ Yes |
| `own<T>` | owned **heap** objects  | ✅ Yes |
| `rfc<T>` | shared **heap** objects | ✅ Yes |

by enforcing this separation, we eliminate common issues such as **dangling references**.

## 3. References (`ref<T>`) vs Pointers (`ptr<T>`)

- `ptr<T>` is used for everything including **heap**, and requires explicit null checking.
- `ref<T>` is restricted to **stack** or **static** memory and guarantees fully pledged safety.

## 4. Reference Scope and Function Return Rules

reference scope is **a concept that restricts returning** `ref<T>`.
a function can only return a reference under specific conditions:

| Type                | Can Return `ref<T>`?             | Reason       |
|-------------------- |--------------------------------- |------------- |
| Method              | ✅ Only for `this` fields        | always safe  |
| Static Method       | ✅ Only for **static** fields    | always safe  |
| Global Function     | ✅ Only for **global** variables | always safe  |
| Heap Memory Objects | ❌ No, must return `ptr<T>`      | dangling ref |

despite returning static field from instance method is safe, it is **not allowed**.
therefore, ensuring good **encapsulation** and consistent & predictable behaviour.

## 5. Inline Functions for Borrowing Multiple Lifetimes

a function returning a `ref<T>` from args **must be marked** `inline` to prevent lifetime ambiguity.

```
@static
@inline
fun! either(a: ref<foo>, b: ref<foo>): ref<utf8>
{
	if (...)
	{
		return &a.name;
	}
	else
	{
		return &b.name;
	}
}

...

either(a, b); // ✅
```

> **Q**: "Why is it necessary?"

- it ensures that `a` or `b` remains **alive at the call site**, preventing dangling refs.
- the compiler **cannot guarantee that one of the inputs hasn’t been dropped before use**.

## 6. Language Features Overview

a brief overview of **MOE**'s core language features.

| Keyword  | Description      |
|--------- |----------------- |
| `fun`    | function         |
| `fun!`   | pure function    |
| `let`    | variable         |
| `let!`   | const variable   |

access modifiers are set using prefixes.

| Prefix  | Description  |
|-------- |------------- |
| *none*  | public       |
| `_`     | private      |
| `$`     | protected    |

and more to come...
