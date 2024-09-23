module.exports = {
    extends: ['@commitlint/config-conventional'],
    rules: {
      'header-pattern': [2, 'always', /^Issue #[0-9]+: .+/],
      'header-match-team': [2, 'always'],
    },
    plugins: [
      {
        rules: {
          'header-match-team': ({header}) => {
            const issuePattern = /^Issue #([0-9]+): /;
            const match = header.match(issuePattern);
  
            if (!match) {
              return [false, 'Commit message must start with "Issue #[n]:", where n is the issue number'];
            }
  
            return [true, ''];
          },
        },
      },
    ],
  };
  