<!-- Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors. MIT License. -->
# Contributing

Thanks for wanting to help. This project is just starting, so there is a lot of room for ideas,
tests and fixes. Please read the [Code of Conduct](CODE_OF_CONDUCT.md) first. It's short.

## The best way to help right now

**Run the probes and tell us what you see.** Windows acts a bit differently from build to build,
and we only have a few machines. Your results help more than code at this stage.

Open an issue and include:

- Your Windows version and build number (run `winver`).
- The output of `shell_probe`.
- If you ran `tray_probe`: what it printed, and which app you used to make a tray icon.
- Whether Explorer was running as the shell.

Please remove anything private first, like your user name in file paths.

## Other ways to help

- Report a bug or a wrong guess in the docs. Parts of `docs/architecture.md` are marked
  "not checked yet". If you know the answer, tell us.
- Suggest an idea. Open an issue and say what problem it solves.
- Fix a typo or make the docs easier to read.
- Write code. See below.

## Before you write a lot of code

Open an issue first and say what you want to do. This saves you time if we're already working on
it, or if the plan needs to change. Small fixes can go straight to a pull request.

## Test safely

This project changes how Windows starts your desktop. Never test shell replacement on your main
account. Use a spare local user or a VM. The steps, and how to get back if it breaks, are in
[docs/dev-environment.md](docs/dev-environment.md).

## Building

You need Windows 10 or 11, Visual Studio 2022 (or just the Build Tools), and CMake 3.24 or newer.

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Code

- Match the style of the code around you.
- Keep comments short. Say why, not what.
- Keep Windows code in this repo's own folders. Change upstream Noctalia code as little as you can.
  If a change would help Noctalia on every system, we'll send it upstream to the
  [noctalia-dev](https://github.com/noctalia-dev) organization instead.
- Use C++20.
- Put this line at the top of every new file, using the comment style of the language:

  ```cpp
  // Copyright (c) 2026 Damian Daniel <damiand@fedoraproject.org> and contributors
  // SPDX-License-Identifier: MIT
  ```

## Pull requests

- Do one thing per pull request. Small ones are easier to review.
- Say what you changed and why.
- Say how you tested it, and on which Windows build.
- Make sure CI passes.
- Be patient. This is a side project, and replies may take a few days.

For commit messages, a short first line in the present tense works well, like
`Add tray message dump to tray_probe`.

## License

By sending a change, you agree that it can be shared under the [MIT license](LICENSE).

## Questions

Open an issue. There are no silly questions here, and the answer will probably help the next person.
