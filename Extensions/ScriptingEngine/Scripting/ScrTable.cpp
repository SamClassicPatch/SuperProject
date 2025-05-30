/* Copyright (c) 2025 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "StdH.h"

namespace sq {

void TableBase::SetTable(const SQChar *strName, const TableBase &objTable) {
  sq_pushobject(m_vm, m_obj); // Push table

  sq_pushstring(m_vm, strName, -1);
  sq_pushobject(m_vm, objTable.GetObj());
  sq_newslot(m_vm, -3, SQFalse);

  sq_poptop(m_vm); // Pop table
};

Table TableBase::AddTable(const SQChar *strName, bool bStatic) {
  sq_pushobject(m_vm, m_obj); // Push table
  sq_pushstring(m_vm, strName, -1);

  HSQOBJECT objTable;
  sq_newtable(m_vm);
  sq_getstackobj(m_vm, -1, &objTable);
  Table table(m_vm, objTable);

  sq_newslot(m_vm, -3, bStatic ? SQTrue : SQFalse);
  sq_poptop(m_vm); // Pop table

  return table;
};

}; // namespace
