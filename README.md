# s2re2

The Google RE2 regular expression library packaged as a SingleStore extension! You might find this extension useful if
you need to execute regular expressions containing user input, as RE2 is designed with
[safety in mind](https://github.com/google/re2/wiki/WhyRE2).

Benefits include:

- Linear execution time for all inputs (safe to use with untrusted input).
- Convenient method to extract a substring using a regular expression.
- Faster than the built-in REGEXP for some patterns (related to #1).

For more on the creation of this extension, see [www.jasonthorsness.com/12](https://www.jasonthorsness.com/12)

## Installation

Install or upgrade `s2re2` on SingleStore 8.5 or higher with the following SQL. This method of installation (extension +
wrappers) enables zero-downtime upgrade or rollback.

```sql
-- versioned s2re2 extension
CREATE EXTENSION s2re2_2024_08_18
FROM HTTP
'https://github.com/jasonthorsness/s2re2/raw/main/dist/2024_08_18.tar';

-- s2re2_match
DELIMITER //
CREATE OR REPLACE FUNCTION s2re2_match(
  input LONGTEXT NULL,
  pattern LONGTEXT NULL)
  RETURNS TINYINT(1) NULL AS
BEGIN
  RETURN s2re2_2024_08_18_match(input, pattern);
END //
DELIMITER ;

-- s2re2_match_extract
DELIMITER //
CREATE OR REPLACE FUNCTION s2re2_match_extract(
  input LONGTEXT NOT NULL,
  pattern LONGTEXT NOT NULL)
  RETURNS LONGTEXT AS
BEGIN
  RETURN s2re2_2024_08_18_match_extract(input, pattern);
END //
DELIMITER ;
```

## Usage

The extension includes two commands, `s2re2_match` and `s2re2_match_extract`.

These functions are optimized for use with a constant pattern, like `SELECT s2re2_match(col, 'foo.+bar') FROM tbl`. If
the pattern used varies across rows (like the pattern itself is stored in a column) execution will be slower.

Setup for the examples below:

```sql
CREATE TABLE tbl(col LONGTEXT NOT NULL);
INSERT INTO tbl VALUES
("https://www.singlestore.com/cloud-trial/"),
("Visit http://weather.bingo/901846663/71273/threeday/f for Seattle weather!"),
("No URL; something.else");
```

### `s2re2_match`

`s2re2_match` takes an input string and a pattern. If either is NULL, it returns NULL. Otherwise, it returns 1 if the
pattern matches the input string and 0 if there is no match. It is a replacement for `REGEXP`.

Example:

```sql
-- find rows containing URLs
SELECT col FROM tbl WHERE s2re2_match(col, 'https?://[^/]+');

-- "Visit http://weather.bingo/901846663/71273/threeday/f for Seattle weather!"
-- "https://www.singlestore.com/cloud-trial/"
```

### `s2re2_match_extract`

`s2re2_match_extract` takes an input string and a pattern containing at least one capture group. If the input string or
pattern is NULL, or the pattern does not contain a capture group, it returns NULL. Otherwise, if the pattern matches the
input, it returns the contents of the first capture group, and if the pattern does not match the input, it returns NULL.

The entire match can be extracted by surrounding the entire pattern in parenthesis, making the whole pattern a capture
group.

Examples:

```sql
-- extract whole URL from text
SELECT col, s2re2_match_extract(col, '(https?://[^/]+[^\\s]*)') AS url
FROM tbl
WHERE NOT ISNULL(url)

-- https://www.singlestore.com/cloud-trial/
-- http://weather.bingo/901846663/71273/threeday/f
```

```sql
-- extract just the host from a url in text
SELECT col, s2re2_match_extract(col, 'https?://([^/]+)') AS host
FROM tbl
WHERE NOT ISNULL(host)

-- weather.bingo
-- www.singlestore.com
```

## Development

To build, you'll need to have Docker installed and a Linux-compatible shell. The build process is performed within a
Docker image -- see the [/docker](/docker/) folder for sources and the Dockerfile.

To build and publish, run [build.sh](/build.sh).

Edit the version at the top of build.sh to make a new version.

## Testing

After building, you can install by running the contents of /build/install.sql. To do this easily locally, run the docker
version of SingleStore

```
docker run --rm -d \
 --name singlestoredb-dev \
 -e ROOT_PASSWORD="password123" \
 -p 3306:3306 -p 8080:8080 -p 9000:9000 \
 ghcr.io/singlestore-labs/singlestoredb-dev:latest
```

Then browse to https://localhost:8080, enter root/password123, paste the contents of install.sql into the SQL editor,
and run it.

## Publishing

The build process produces a .tar file under [/dist](/dist/); this is ready for users to install as an extension. It can
be installed directly from GitHub or the file can be uploaded to any other location or just passed as a base64 string
(see the
[documentation](https://docs.singlestore.com/cloud/reference/sql-reference/procedural-sql-reference/extensions/) for
more options).
