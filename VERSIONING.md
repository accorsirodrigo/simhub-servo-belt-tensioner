# Sistema de Versionamento - Belt Tensioner

## Controle de Versão

O projeto utiliza **Semantic Versioning** (SemVer) no formato `MAJOR.MINOR.PATCH`.

### Arquivo de Controle

A versão atual está armazenada em `version.json`:

```json
{
  "major": 1,
  "minor": 0,
  "patch": 0,
  "version": "1.0.0"
}
```

## Releases Automáticas

Uma nova release é criada automaticamente quando:

1. Um commit é mergeado na branch `main`
2. Contém alterações em:
   - `Arduino/Belt-tensioner.ino`
   - `Simhub/custom-device-belt-tensioner.shsds`

## Determinando o Tipo de Bump

O tipo de versionamento é determinado pela **label do Pull Request**:

| Label | Bump Type | Resultado |
|-------|-----------|-----------|
| `major` | Incrementa versão principal | `1.0.0` → `2.0.0` |
| `minor` | Incrementa versão secundária | `1.0.0` → `1.1.0` |
| `patch` | Incrementa correção | `1.0.0` → `1.0.1` |
| *(nenhuma)* | Padrão: patch | `1.0.0` → `1.0.1` |

### Como Usar

1. **Criar um Pull Request** com alterações nos arquivos monitorados
2. **Adicionar uma label** ao PR:
   - Clique em "Labels" no PR
   - Selecione `major`, `minor`, ou `patch`
3. **Fazer merge** do PR para `main`
4. O workflow automático irá:
   - Determinar a nova versão
   - Atualizar `version.json`
   - Criar uma tag Git (`v1.0.1`)
   - Criar uma Release com o arquivo ZIP

## Release ZIP

O arquivo de release é nomeado como `belt-tensioner-X.Y.Z.zip` e contém:

```
belt-tensioner-1.0.0.zip
├── Arduino/
│   └── Belt-tensioner.ino
└── Simhub/
    └── custom-device-belt-tensioner.shsds
```

## Workflow: `.github/workflows/auto-release.yml`

O workflow é acionado automaticamente em push para `main` quando há alterações nos arquivos monitorados.

### O que acontece:

1. ✅ Detecta o PR associado ao commit
2. ✅ Lê as labels (`major`, `minor`, `patch`)
3. ✅ Calcula a nova versão
4. ✅ Atualiza `version.json`
5. ✅ Faz commit e push da versão
6. ✅ Cria tag Git (`vX.Y.Z`)
7. ✅ Cria Release no GitHub com o ZIP

## Exemplo

### Cenário: Atualizar Arduino firmware

1. Editar `Arduino/Belt-tensioner.ino`
2. Fazer commit e criar PR
3. Adicionar label `minor` (nova funcionalidade)
4. Merge para `main`
5. **Resultado:**
   - Versão: `1.0.0` → `1.1.0`
   - Tag: `v1.1.0`
   - Release: `belt-tensioner-1.1.0.zip`

## Notas Importantes

- As labels devem estar **em minúsculas** (case-insensitive para segurança)
- Se múltiplas labels de versão forem adicionadas, prevalece a maior (major > minor > patch)
- O commit de atualização de versão é feito automaticamente pelo bot
- Tags são criadas e pushadas automaticamente
