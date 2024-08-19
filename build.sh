#!/bin/bash
set -e

# Update to create a new version
VERSION=2024_08_18

rm -rf ./build
mkdir -p ./build
docker build -t s2re2_$VERSION ./docker/
docker create --name s2re2_$VERSION s2re2_$VERSION
docker cp s2re2_$VERSION:/match.wasm ./build/match.wasm
docker cp s2re2_$VERSION:/match.txt ./build/match.txt
docker cp s2re2_$VERSION:/match_extract.wasm ./build/match_extract.wasm
docker cp s2re2_$VERSION:/match_extract.txt ./build/match_extract.txt
docker rm s2re2_$VERSION

echo "CREATE FUNCTION s2re2_${VERSION}_match_inner(
    input LONGTEXT NOT NULL,
    pattern LONGTEXT NOT NULL)
    RETURNS TINYINT(1) NOT NULL
    AS WASM 
    FROM LOCAL INFILE 'match.wasm'
    USING EXPORT 'Match';

DELIMITER //
CREATE FUNCTION s2re2_${VERSION}_match(
    input LONGTEXT,
    pattern LONGTEXT)
RETURNS TINYINT(1) AS
DECLARE
    result RECORD(m TINYINT(1) NOT NULL, s LONGTEXT NOT NULL);
BEGIN
    IF ISNULL(input) OR ISNULL(pattern) THEN
        RETURN NULL;
    END IF;
    return s2re2_${VERSION}_match_inner(input, pattern);
END //
DELIMITER ;

CREATE FUNCTION s2re2_${VERSION}_match_extract_inner(
    input LONGTEXT NOT NULL,
    pattern LONGTEXT NOT NULL)
    RETURNS RECORD(m TINYINT(1) NOT NULL, s LONGTEXT NOT NULL)
    AS WASM 
    FROM LOCAL INFILE 'match_extract.wasm'
    USING EXPORT 'MatchExtract';
   
DELIMITER //
CREATE FUNCTION s2re2_${VERSION}_match_extract(
    input LONGTEXT NULL,
    pattern LONGTEXT NULL)
RETURNS LONGTEXT AS
DECLARE
    result RECORD(m TINYINT(1) NOT NULL, s LONGTEXT NOT NULL);
BEGIN
    IF ISNULL(input) OR ISNULL(pattern) THEN
        RETURN NULL;
    END IF;
    result = s2re2_${VERSION}_match_extract_inner(input, pattern);
    IF result.m = 0 THEN
        RETURN NULL;
    ELSE
        RETURN result.s;
    END IF;
END //
DELIMITER ;
" \
> ./build/s2re2_${VERSION}.sql

tar -cf ./dist/${VERSION}.tar -C ./build/ \
match.wasm match_extract.wasm s2re2_${VERSION}.sql

BASE64=$(base64 -w 0 ./dist/${VERSION}.tar)

echo "
DROP EXTENSION IF EXISTS s2re2_${VERSION};
CREATE EXTENSION s2re2_${VERSION} FROM BASE64 '$BASE64';

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
" \
> ./build/install.sql
