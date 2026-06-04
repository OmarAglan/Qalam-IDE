# Icon Resource Checklist (Qalam IDE)

This project uses Qt resources under `qalam/resources.qrc` and icon paths like:

`:/icons/resources/<name>.svg`

## Verify no missing icon resources

From repo root:

```sh
# List all icon resource paths referenced by code/QSS
rg -n \"\\:/icons/resources/\" -S qalam source documents --glob '!qalam/build/**' \\
  | sed -n 's/.*\\(:\\/icons\\/resources\\/[^\"\\) ]*\\).*/\\1/p' \\
  | sort -u

# List all resources registered in .qrc
rg -n \"<file>resources/\" qalam/resources.qrc \\
  | sed -n 's/.*<file>\\(resources\\/[^<]*\\)<\\/file>.*/\\1/p' \\
  | sort -u
```

## Verify PNG icons are not used (except app logo assets)

```sh
rg -n \"\\.(png|ico)\\b\" -S qalam source documents --glob '!qalam/build/**'
```

