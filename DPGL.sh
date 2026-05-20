#!/usr/bin/env bash
# Toggle C/C++ suggestion settings in .vscode/settings.json
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SETTINGS="$ROOT_DIR/.vscode/settings.json"
BACKUP="$SETTINGS.bak.$(date +%s)"

if [ ! -f "$SETTINGS" ]; then
  echo "No workspace settings.json found at $SETTINGS"
  exit 1
fi

cp "$SETTINGS" "$BACKUP"
echo "Backed up settings to $BACKUP"

python3 - "$SETTINGS" <<'PY'
import json,sys
path = sys.argv[1]
with open(path,'r',encoding='utf-8') as f:
    data = json.load(f)

# Determine current state: consider C_Cpp.intelliSenseEngine == 'Disabled' as disabled
disabled = data.get('C_Cpp.intelliSenseEngine','') == 'Disabled'

def set_disabled(d):
    if d:
        data['C_Cpp.intelliSenseEngine'] = 'Disabled'
        data['C_Cpp.intelliSenseEngineFallback'] = 'Disabled'
        data['C_Cpp.autocomplete'] = 'Disabled'
        data['intellicode.completionsEnabled'] = False
        data['intellicode.completions.enabled'] = False
        data.setdefault('[c]',{})
        data.setdefault('[cpp]',{})
        data['[c]'].update({
            'editor.quickSuggestions': False,
            'editor.suggestOnTriggerCharacters': False,
            'editor.wordBasedSuggestions': False,
            'editor.acceptSuggestionOnEnter': 'off',
            'editor.parameterHints.enabled': False,
            'editor.inlineSuggest.enabled': False
        })
        data['[cpp]'].update({
            'editor.quickSuggestions': False,
            'editor.suggestOnTriggerCharacters': False,
            'editor.wordBasedSuggestions': False,
            'editor.acceptSuggestionOnEnter': 'off',
            'editor.parameterHints.enabled': False,
            'editor.inlineSuggest.enabled': False
        })
    else:
        # enable defaults
        data['C_Cpp.intelliSenseEngine'] = 'Default'
        data['C_Cpp.intelliSenseEngineFallback'] = 'Enabled'
        data['C_Cpp.autocomplete'] = 'Default'
        data['intellicode.completionsEnabled'] = True
        data['intellicode.completions.enabled'] = True
        data.setdefault('[c]',{})
        data.setdefault('[cpp]',{})
        data['[c]'].update({
            'editor.quickSuggestions': True,
            'editor.suggestOnTriggerCharacters': True,
            'editor.wordBasedSuggestions': True,
            'editor.acceptSuggestionOnEnter': 'on',
            'editor.parameterHints.enabled': True,
            'editor.inlineSuggest.enabled': True
        })
        data['[cpp]'].update({
            'editor.quickSuggestions': True,
            'editor.suggestOnTriggerCharacters': True,
            'editor.wordBasedSuggestions': True,
            'editor.acceptSuggestionOnEnter': 'on',
            'editor.parameterHints.enabled': True,
            'editor.inlineSuggest.enabled': True
        })

if disabled:
    set_disabled(False)
    new_state = 'ENABLED'
else:
    set_disabled(True)
    new_state = 'DISABLED'

with open(path,'w',encoding='utf-8') as f:
    json.dump(data,f,indent=2,ensure_ascii=False)

print(new_state)
PY

echo "C/C++ suggestions toggled. Reload VS Code to apply changes."
