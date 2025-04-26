# Contributing to OceanOS

Thank you for your interest in contributing to OceanOS!

To maintain the consistency and vision of the project, please follow these guidelines:

---

## Language Policy

- All communications (Pull Requests, Issues, Discussions) must be written in **English**.
- Optionally, **Spanish** may be used for comments and documentation alongside English.

---

## Supported Technologies

OceanOS strictly supports only **UEFI-based modern architectures**.  
Legacy technologies such as BIOS, VGA direct memory access, PS/2 keyboard interfaces, and ATA storage are **not allowed**.

---

## Programming Languages

OceanOS allows only specific languages depending on the component:

| **Component**       | **Allowed Languages**                                |
|:--------------------|:-----------------------------------------------------|
| **Bootloader**       | C, Rust, Assembly                                     |
| **Kernel**           | C, Rust                                               |
| **Drivers**          | C, Rust, Zig<br>(Carbon will be considered later)     |
| **Build & Scripts**  | Make, Python, Cargo                                   |

---

- **Bootloader**: Only C, Rust, and Assembly are allowed for maximum control and minimal dependencies.
- **Kernel**: C and Rust are the only accepted languages to ensure performance and safety.
- **Drivers**: C and Rust are allowed; Zig is accepted for modern low-level drivers. Carbon may be allowed once it stabilizes.
- **Build & Scripts**: Only Makefiles, Python scripts, and Cargo (Rust’s package manager) are permitted.

---

## Code Style

### Code Formatting

- All code **must strictly follow the K&R (Kernighan & Ritchie) style.**
- Consistent formatting across the entire codebase is mandatory.

#### ✅ K&R Style Example (Correct)

```c
int main(void) {
    printf("Hello, OceanOS!\n");
    return 0;
}

void example_function(int a, int b) {
    if (a > b) {
        printf("A is greater\n");
    } else {
        printf("B is greater or equal\n");
    }
}
```

K&R style keeps the opening brace `{` on the same line as the function or control statement, improving compactness and readability.

---

#### ❌ Incorrect Style Example (Allman Style — Not Allowed)

```c
int main(void)
{
    printf("Hello, OceanOS!\n");
    return 0;
}

void example_function(int a, int b)
{
    if (a > b)
    {
        printf("A is greater\n");
    }
    else
    {
        printf("B is greater or equal\n");
    }
}
```

In Allman style, the opening brace `{` is placed on a new line, which is not allowed in OceanOS.

---

### Naming Conventions (Based on Rust Style)

All naming must follow the `Rust style guidelines`, except in cases where the programming language syntax prevents it.

| Element               | Naming Convention | Example                       |
|-----------------------|-------------------|-------------------------------|
| Functions / Variables | snake_case        | read_file(), current_status   |
| Types / Structs       | PascalCase        | FileReader, NetworkPacket     |
| Macros / Constants    | ALL_CAPS          | MAX_BUFFER_SIZE, DEFAULT_PORT |

* Always use lowercase for functions and variables with underscores between words.

* Always use uppercase starting letters without underscores for types and structs.

* Always use full uppercase with underscores for macros and constants.

### Special Cases
If a specific language **syntax limitation** prevents following these rules (e.g., Assembly syntax constraints), exceptions may be made on a case-by-case basis.

---

## Pull Requests

- Write clear and meaningful commit messages and PR titles.
- Keep your PR focused — avoid mixing unrelated changes.
- Describe what you have changed and why in the PR description.
- Follow the project's technical standards and architecture principles.

---

## Communication

- Be respectful, professional, and constructive.
- All contributors are expected to maintain a collaborative and open attitude.
- Questions are welcome; it is encouraged to ask for clarification when needed.

---

Thank you for helping build OceanOS into a fully modern and open-source operating system!
